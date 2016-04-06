using System;
using System.IO;
using System.Threading.Tasks;
using Windows.Devices.Enumeration;
using Windows.Devices.I2c;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;

namespace RPiViewer {
	/// <summary>
	/// This page is displayed when the application starts up.
	/// TODO Replace with Background Service once fully debugged
	/// </summary>
	public sealed partial class MainPage : Page {
		/// <summary>
		/// The default I2C controller ID.
		/// </summary>
		public const string I2C_CONTROLLER = "I2C1";
		/// <summary>
		/// The Tiva C slave address.
		/// </summary>
		public const int SLAVE_ADDRESS = 0x2A;
		/// <summary>
		/// The major version compliant with this version.
		/// </summary>
		public const int SW_VERSION = 2;

		/// <summary>
		/// The ID of the ACPI device matching "I2C1".
		/// </summary>
		private string i2cID;

		/// <summary>
		/// Default constructor.
		/// </summary>
		public MainPage() {
			InitializeComponent();
			i2cID = null;
		}

		/// <summary>
		/// Queries the system and populates i2cID with the ACPI ID of the I2C controller.
		/// </summary>
		/// <param name="controller">The controller ID to match</param>
		private async void FindI2CID(string controller) {
			// Find device information
			var ds = I2cDevice.GetDeviceSelector(controller);
			var info = await DeviceInformation.FindAllAsync(ds);
			if (info.Count > 0)
				// Found a device
				i2cID = info[0].Id;
		}

		/// <summary>
		/// Opens a connection to the specified I2C device address.
		/// </summary>
		/// <param name="address">The 7-bit right aligned slave address (should not have 0x80
		/// bit set!)</param>
		/// <returns>A reference to the device</returns>
		/// <exception cref="IOException">If I2C1 cannot be opened</exception>
		private async Task<I2cDevice> OpenI2CConnection(int address) {
			I2cConnectionSettings settings = new I2cConnectionSettings(address);
			// Create settings to address device
			settings.BusSpeed = I2cBusSpeed.FastMode;
			settings.SharingMode = I2cSharingMode.Exclusive;
			if (i2cID == null)
				throw new IOException("Failed to find I2C controller matching " +
					I2C_CONTROLLER);
			return await I2cDevice.FromIdAsync(i2cID, settings);
		}

		/// <summary>
		/// Displays the GPS data on the screen.
		/// </summary>
		/// <param name="device">The I2C device</param>
		/// <param name="index">0 for Venus, 1 for Copernicus</param>
		/// <param name="display">An array containing 4 text boxes - lat, lon, alt, vel</param>
		private void GPSFetch(I2cDevice device, int index, params TextBlock[] display) {
			byte[] data = new byte[17];
			// Receive 17 bytes starting from lat register (0x10), where 0x30 is the second GPS
			byte address = (byte)(((index == 0) ? 0x00 : 0x20) + 0x10);
			device.WriteRead(new byte[] { address }, data);
			// Convert all to correct types
			double lat = BitConverter.ToInt32(data, 0) * 1E-6;
			double lon = BitConverter.ToInt32(data, 4) * 1E-6;
			double vel = BitConverter.ToInt16(data, 8) * 1E-1;
			double head = BitConverter.ToInt16(data, 10) * 1E-1;
			double alt = BitConverter.ToInt32(data, 12) * 1E-2;
			int satellites = (int)data[16];
			// Write to the screen
			if (satellites > 0) {
				display[0].Text = lat.ToString("F6");
				display[1].Text = lon.ToString("F6");
				display[2].Text = alt.ToString("F1");
				display[3].Text = vel.ToString("F1");
			} else {
				display[0].Text = "NO FIX";
				display[1].Text = satellites.ToString();
				display[2].Text = "";
				display[3].Text = "";
			}
		}

		private async void GPSUpdate(object sender, object e) {
			try {
				using (I2cDevice device = await OpenI2CConnection(SLAVE_ADDRESS)) {
					GPSFetch(device, 0, txtLat1, txtLon1, txtAlt1, txtVel1);
					GPSFetch(device, 1, txtLat2, txtLon2, txtAlt2, txtVel2);
				}
			} catch (IOException ex) {
				// Not acknowledged!
				txtLat1.Text = "ERROR";
				txtLon1.Text = ex.Message;
				txtAlt1.Text = "";
				txtVel1.Text = "";
			}
		}

		private async void GPSStartup(object sender, RoutedEventArgs e) {
			bool ok = false;
			byte[] data = new byte[3];
			// Prepare I2C controller
			FindI2CID(I2C_CONTROLLER);
			for (int i = 0; i < 4 && !ok; i++) {
				// Multiple retries to get in sync
				await Task.Delay(100);
				try {
					using (I2cDevice device = await OpenI2CConnection(SLAVE_ADDRESS)) {
						// Stream read 0x00 (WHO_AM_I), 0x01 (SW_VERSION_MAJOR),
						// 0x02 (SW_VERSION_MINOR)
						device.WriteRead(new byte[] { 0x00 }, data);
						if ((data[0] & 0xFF) == SLAVE_ADDRESS && data[1] == SW_VERSION) {
							// Check compatibility with the SW_VERSION
							DispatcherTimer timer = new DispatcherTimer();
							timer.Interval = TimeSpan.FromMilliseconds(100.0);
							timer.Tick += GPSUpdate;
							// Everything is OK -- go ahead
							timer.Start();
							ok = true;
						}
					}
					// Ignore exceptions and try again
				} catch (IOException) { }
			}
			if (!ok) {
				// Connection cannot be found
				txtLat1.Text = "No Connect";
				txtLon1.Text = "Check connections";
				txtAlt1.Text = "PA6 => SCL";
				txtVel1.Text = "PA7 => SDA";
            }
		}
	}
}
