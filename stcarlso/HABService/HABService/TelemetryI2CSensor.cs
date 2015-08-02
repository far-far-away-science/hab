using System;
using System.IO;
using System.Threading.Tasks;
using Windows.Devices.I2c;

namespace HABService {
	/// <summary>
	/// Interface to the Telemetry MCU to log the GPS data from both devices and the
	/// ODTS (if configured)
	/// </summary>
	class TelemetryI2CSensor : I2CSensor {
		/// <summary>
		/// The column guide string used in the log headers.
		/// </summary>
		public const string COLS = "FIX_1,LAT_1,LON_1,ALT_1,VEL_1,HDG_1," +
			"FIX_2,LAT_2,LON_2,ALT_2,VEL_2,HDG_2,TEMP,VOLT";
		/// <summary>
		/// The Tiva C default slave address.
		/// </summary>
		public const int SLAVE_ADDRESS = 0x2A;
		/// <summary>
		/// The major version compliant with this version.
		/// </summary>
		public const int SW_VERSION = 2;

		/// <summary>
		/// The I2C address of the telemetry MCU.
		/// </summary>
		private readonly int address;

		public override int Address {
			get {
				return address;
			}
		}
		/// <summary>
		/// Checking this happens at 2Hz. Since Copernicus only returns at 1Hz there is no
		/// reason to go any faster, the 2x is just to mitigate phasing errors.
		/// </summary>
		public override int LogInterval {
			get {
				return 500;
			}
		}

		/// <summary>
		/// Create a connection to the default Telemetry MCU address.
		/// </summary>
		public TelemetryI2CSensor() : this(SLAVE_ADDRESS) { }
		/// <summary>
		/// Create a connection to a custom Telemetry MCU address.
		/// <param name="address">The custom I2C address</param>
		/// </summary>
		public TelemetryI2CSensor(int address) {
			this.address = address;
		}
		/// <summary>
		/// Displays the GPS data on the screen.
		/// </summary>
		/// <param name="device">The I2C device</param>
		/// <param name="index">0 for Venus, 1 for Copernicus</param>
		/// <returns>The GPS data in a structure</returns>
		private GPSData GPSFetch(I2cDevice device, int index) {
			byte[] data = new byte[17];
			// Receive 17 bytes starting from 0x10, where 0x30 is the second GPS
			byte address = (byte)(((index == 0) ? 0x00 : 0x20) + 0x10);
			device.WriteRead(new byte[] { address }, data);
			GPSData result = new GPSData();
			// Convert all to correct types
			result.latitude = BitConverter.ToInt32(data, 0) * 1E-6;
			result.longitude = BitConverter.ToInt32(data, 4) * 1E-6;
			result.velocity = BitConverter.ToInt16(data, 8) * 1E-1;
			result.heading = BitConverter.ToInt16(data, 10) * 1E-1;
			result.altitude = BitConverter.ToInt32(data, 12) * 1E-2;
			result.satellites = data[16];
			return result;
		}
		public override string Init(I2cDevice device) {
			byte[] data = new byte[3];
			// Stream read 0x00 (WHO_AM_I), 0x01 (SW_VERSION_MAJOR),
			// 0x02 (SW_VERSION_MINOR)
			device.WriteRead(new byte[] { 0x00 }, data);
			// Check for sync
			int sync = data[0] & 0xFF;
			if (sync != SLAVE_ADDRESS)
				throw new IOException(String.Format("Telemetry MCU is not in sync. " +
					"Expected: 0x{0:X2} Received: 0x{1:X2}", SLAVE_ADDRESS, sync));
			// Check firmware version [major only]
			int fw = data[1] & 0xFF;
            if (fw != SW_VERSION)
				throw new IOException(String.Format("Telemetry MCU firmware mismatch. " +
					"Expected: {0:D} Received: {1:D}", SW_VERSION, fw));
			// Column guide
			return COLS;
		}
		public override async Task<String> Sample(I2cDevice device, object gsl) {
			// NOTE This routine reads and reports both GPSes in the log. It should be trivial
			// to select which one you really want in post processing. Even at 2 Hz with 256
			// characters per line and a 20 hour flight, the total file size will be a very
			// manageable 20 MB.
			string result;
			lock (gsl) {
				GPSData venus = GPSFetch(device, 0);
				GPSData copernicus = GPSFetch(device, 1);
				// Temperature and voltage
				byte[] data = new byte[4];
				device.WriteRead(new byte[] { 0x04 }, data);
				// Equation for temperature on p. 813 of TM4C123GHPM datasheet
				float temp = 147.5F - 75F * 3.3F * BitConverter.ToUInt16(data, 0) / 4096F;
				float voltage = BitConverter.ToUInt16(data, 2) * 1E-3F;
				// Voltage is in mV
				result = String.Format("{0},{1},{2:F1},{3:F3}", venus, copernicus, temp, voltage);
			}
			// Give up VS!
			await Task.Delay(0);
			return result;
		}
	}

	/// <summary>
	/// A structure that holds GPS data.
	/// </summary>
	struct GPSData {
		/// <summary>
		/// Satellite count visible, or 0 if no fix is available.
		/// </summary>
		public int satellites;
		/// <summary>
		/// Latitude in signed decimal degrees.
		/// </summary>
		public double latitude;
		/// <summary>
		/// Longitude in signed decimal degrees.
		/// </summary>
		public double longitude;
		/// <summary>
		/// Velocity over ground in km/h.
		/// </summary>
		public double velocity;
		/// <summary>
		/// True (i.e. not magnetic) heading made good in degrees (0-360).
		/// </summary>
		public double heading;
		/// <summary>
		/// Altitude in meters.
		/// </summary>
		public double altitude;

		public override string ToString() {
			return String.Format("{0:D},{1:F6},{2:F6},{3:F1},{4:F1},{5:F1}", satellites,
				latitude, longitude, altitude, velocity, heading);
		}
	}
}
