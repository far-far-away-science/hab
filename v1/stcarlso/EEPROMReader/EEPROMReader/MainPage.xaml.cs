#define NO_MOUSE

using System;
using System.IO;
using System.Threading.Tasks;
using Windows.Devices.Enumeration;
using Windows.Devices.I2c;
using Windows.Storage;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Media;

namespace EEPROMReader {
	/// <summary>
	/// A basic application for reading out the EEPROM of the HAB device over I2C.
	/// </summary>
	public sealed partial class MainPage : Page {
		/// <summary>
		/// The default I2C controller ID.
		/// </summary>
		public const string I2C_CONTROLLER = "I2C1";
		/// <summary>
		/// Register address for "Who am I" (sync/address check)
		/// </summary>
		public const int REG_WHO_AM_I = 0x00;
		/// <summary>
		/// Register address for EEPROM address pointer.
		/// </summary>
		public const int REG_EEADDR = 0x08;
		/// <summary>
		/// Register address for EEPROM data.
		/// </summary>
		public const int REG_EEDATA = 0x0A;
		/// <summary>
		/// The Tiva C default slave address.
		/// </summary>
		public const int SLAVE_ADDRESS = 0x2A;
		/// <summary>
		/// The major version compliant with this software.
		/// </summary>
		public const int SW_VERSION_MAJOR = 2;
		/// <summary>
		/// The minor version compliant with this software.
		/// </summary>
		public const int SW_VERSION_MINOR = 2;

#if NO_MOUSE
		/// <summary>
		/// Auto read countdown to work around bugs with input devices in IoT Core.
		/// </summary>
		private int autoRead;
		/// <summary>
		/// Counts down with a 1s interval for the auto read.
		/// </summary>
		private DispatcherTimer timer;
#endif

		public MainPage() {
			InitializeComponent();
		}
		/// <summary>
		/// Converts raw bytes to the true voltage and temperature readings.
		/// </summary>
		/// <param name="data">The data read from the telemetry MCU</param>
		/// <param name="offset">The offset of the log entry, little endian; 2 bytes are
		/// required to be available in the array after this index</param>
		/// <param name="volt">The voltage output in volts</param>
		/// <param name="temp">The temperature output in degrees C</param>
		/// <returns>true if converted, or false if this slot is empty</returns>
		private bool Convert(byte[] data, int offset, out float volt, out float temp) {
			int rawVolt = data[1] & 0xFF, rawTemp = data[0] & 0xFF;
			bool valid = false;
			if (rawVolt != 0xFF || rawTemp != 0xFF) {
				// Format is documented in the telemetry MCU code
				// It is ugly but optimized for dynamic range
				//  Voltage [true] = data * 20 + 5000
				//  Temperature [true] = 147.5 - 0.0604248046875 * (data * 10 + 1600)
				// Data is also packed with 2 log entries per word
				volt = 5.0F + rawVolt * 0.02F;
				temp = 50.8203125F - rawTemp * 0.604248046875F;
				valid = true;
			} else {
				volt = 0.0F;
				temp = 0.0F;
			}
			return valid;
		}
		private void CountdownAutoRead(object sender, object e) {
#if NO_MOUSE
			autoRead--;
			if (autoRead == 0) {
				// Do it
				btnRead_Click(sender, null);
				btnRead.Content = "Read EEPROM";
			} else
				// Let them know
				btnRead.Content = "Read EEPROM (" + autoRead + ")";
#endif
		}
		/// <summary>
		/// Displays an error in the status window.
		/// </summary>
		/// <param name="message">The message to display</param>
		private void DisplayError(string message) {
			lblStatus.Text = message;
			lblStatus.Foreground = (Brush)Application.Current.Resources["BrushError"];
		}
		/// <summary>
		/// Displays a message in the status window.
		/// </summary>
		/// <param name="message">The message to display</param>
		private void DisplayStatus(string message) {
			lblStatus.Text = message;
			lblStatus.Foreground = (Brush)Application.Current.Resources["BrushSuccess"];
		}
		/// <summary>
		/// Queries the system and determines the ACPI ID of the I2C controller.
		/// </summary>
		/// <param name="controller">The controller ID to match</param>
		/// <returns>the ID, or null if no matching controller is found</returns>
		private async Task<string> FindI2CID(string controller) {
			// Find device information
			var ds = I2cDevice.GetDeviceSelector(controller);
			var info = await DeviceInformation.FindAllAsync(ds);
			string i2cID = null;
			if (info.Count > 0)
				// Found a device
				i2cID = info[0].Id;
			return i2cID;
		}
		/// <summary>
		/// Formats a log entry.
		/// </summary>
		/// <param name="index">The index of the entry</param>
		/// <param name="volt">The voltage [V]</param>
		/// <param name="temp">The temperature [C]</param>
		/// <returns>A string describing the log entry, no new line</returns>
		private string FormatLog(int index, float volt, float temp) {
			return String.Format("{0:D},{1:F2},{2:F1}", index, volt, temp);
		}
		/// <summary>
		/// Initializes the telemetry MCU. Checks version and sync bytes to ensure that EEPROM
		/// storage is supported on the firmware currently running.
		/// </summary>
		/// <param name="device">The I2cDevice to initialize</param>
		private void Init(I2cDevice device) {
			byte[] data = new byte[3];
			// Stream read 0x00 (WHO_AM_I), 0x01 (SW_VERSION_MAJOR),
			// 0x02 (SW_VERSION_MINOR)
			device.WriteRead(new byte[] { REG_WHO_AM_I }, data);
			// Check for sync
			int sync = data[0] & 0xFF;
			if (sync != SLAVE_ADDRESS)
				throw new IOException(String.Format("Telemetry MCU is not in sync. " +
					"Expected: 0x{0:X2} Received: 0x{1:X2}", SLAVE_ADDRESS, sync));
			// Check firmware version
			int fwMajor = data[1] & 0xFF;
			if (fwMajor < SW_VERSION_MAJOR)
				throw new IOException(String.Format("Telemetry MCU firmware mismatch. " +
					"Expected: >={0:D} Received: {1:D}", SW_VERSION_MAJOR, fwMajor));
			int fwMinor = data[2] & 0xFF;
			if (fwMinor < SW_VERSION_MINOR && fwMajor == SW_VERSION_MAJOR)
				throw new IOException(String.Format("Telemetry MCU firmware mismatch. " +
					"Expected: >={0:D} Received: {1:D}", SW_VERSION_MINOR, fwMinor));
		}
		/// <summary>
		/// Reads data from the specified EEPROM index. There are 512 indices (512x4 bytes)
		/// so data must be at least 4 bytes.
		/// </summary>
		/// <param name="device">The I2cDevice which has EEPROM data</param>
		/// <param name="index">The index to read</param>
		/// <returns>The data read</returns>
		private byte[] Read(I2cDevice device, int index) {
			byte[] eeData = new byte[4];
			// Set address
			device.WriteRead(new byte[] { REG_EEADDR, (byte)(index & 0xFF),
				(byte)((index & 0x01) >> 8) }, eeData);
			return eeData;
		}
		/// <summary>
		/// Opens a connection to the specified I2C device address.
		/// </summary>
		/// <param name="i2cID">The ID of the I2C controller, from FindI2CID()</param>
		/// <param name="address">The 7-bit right aligned slave address (should not have 0x80
		/// bit set!)</param>
		/// <returns>A reference to the device</returns>
		/// <exception cref="IOException">If the I2C device cannot be opened</exception>
		private async Task<I2cDevice> OpenI2CConnection(string i2cID, int address) {
			I2cConnectionSettings settings = new I2cConnectionSettings(address);
			// Create settings to address device
			settings.BusSpeed = I2cBusSpeed.FastMode;
			settings.SharingMode = I2cSharingMode.Shared;
			if (i2cID == null)
				throw new IOException("Failed to find I2C controller matching " +
					I2C_CONTROLLER);
			return await I2cDevice.FromIdAsync(i2cID, settings);
		}
		/// <summary>
		/// Opens the log file.
		/// </summary>
		/// <returns>The log file</returns>
		/// <exception cref="IOException">If the log could not be opened</exception>
		private async Task<StreamWriter> OpenLog() {
			StorageFile file = await DownloadsFolder.CreateFileAsync("EEPROM.log",
				CreationCollisionOption.GenerateUniqueName);
			return new StreamWriter(await file.OpenStreamForWriteAsync());
		}
		private async void btnRead_Click(object sender, RoutedEventArgs e) {
#if NO_MOUSE
			if (timer != null)
				timer.Stop();
#endif
			try {
				byte[] address = new byte[1];
				// Open connection to telemetry MCU
				string id = await FindI2CID(I2C_CONTROLLER);
				using (I2cDevice telemetryMCU = await OpenI2CConnection(id, SLAVE_ADDRESS)) {
					// Initialize and check FW
					Init(telemetryMCU);
					int offset = 0, index = 0;
					bool end = false;
					float volt, temp;
					using (StreamWriter log = await OpenLog()) {
						// Readout data
						do {
							DisplayStatus(String.Format("Reading entry {0:D}", index));
							byte[] data = Read(telemetryMCU, offset++);
							// Convert the first entry
							if (Convert(data, 0, out volt, out temp)) {
								await log.WriteLineAsync(FormatLog(index++, volt, temp));
							} else
								end = true;
							// Convert the second entry
							if (Convert(data, 2, out volt, out temp)) {
								await log.WriteLineAsync(FormatLog(index++, volt, temp));
							} else
								end = true;
							// Allow screen to update
							await Task.Delay(15);
						} while (!end);
					}
					DisplayStatus(String.Format("Read {0:D} data points from EEPROM", index));
				}
			} catch (Exception ex) {
				DisplayError("Error reading EEPROM: " + ex.Message);
			}
		}

		private void StartCountdown(object sender, RoutedEventArgs e) {
#if NO_MOUSE
			timer = new DispatcherTimer();
			timer.Interval = TimeSpan.FromSeconds(1);
			timer.Tick += CountdownAutoRead;
			autoRead = 10;
			timer.Start();
#endif
		}
	}
}
