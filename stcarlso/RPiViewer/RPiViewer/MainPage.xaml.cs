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
		/// The ID of the ACPI device matching "I2C1".
		/// </summary>
		private string i2cID;

		/// <summary>
		/// Default constructor.
		/// </summary>
		public MainPage() {
			this.InitializeComponent();
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

		private async void GPSUpdate(object sender, object e) {
			try {
				using (I2cDevice device = await OpenI2CConnection(SLAVE_ADDRESS)) {
					// Receive 8 bytes starting from lat register (0x04)
					byte[] data = new byte[8];
					device.WriteRead(new byte[] { 0x04 }, data);
					double lat = (double)BitConverter.ToInt32(data, 0) * 1E-6;
					double lon = (double)BitConverter.ToInt32(data, 4) * 1E-6;
					// Write to the screen
					txtLat.Text = lat.ToString("F6");
					txtLon.Text = lon.ToString("F6");
				}
			} catch (IOException ex) {
				// Not acknowledged!
				txtLat.Text = "ERROR";
				txtLon.Text = ex.Message;
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
						if ((data[0] & 0xFF) == SLAVE_ADDRESS && data[1] == 1) {
							// Compatible with major version 1
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
				txtLat.Text = "No Connect";
				txtLon.Text = "Check connections to Telemetry MCU\r\nPA6 => SCL; PA7 => SDA";
			}
		}
	}
}
