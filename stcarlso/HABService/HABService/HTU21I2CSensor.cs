using System;
using System.IO;
using System.Threading.Tasks;
using Windows.Devices.I2c;

namespace HABService {
	class HTU21I2CSensor : I2CSensor {
		/// <summary>
		/// Conversion for relative humidity = 125 / 65536 (datasheet pg. 15)
		/// </summary>
		private const double HUMID_FACTOR = 0.0019073486328125;
		/// <summary>
		/// Humidity offset (datasheet pg. 15)
		/// This allows -6 and +118% RH, as noted in datasheet. We will not clamp the values,
		/// as cutting the junk data can be done in post processing, and we want to see if
		/// this data actually appears in the log.
		/// </summary>
		private const double HUMID_OFFSET = -6.00;
		/// <summary>
		/// Measures temperature, no hold master (datasheet pg. 11)
		/// </summary>
		private const int MEASURE_TEMP = 0xF3;
		/// <summary>
		/// Measures humidity, no hold master (datasheet pg. 11)
		/// </summary>
		private const int MEASURE_HUMID = 0xF5;
		/// <summary>
		/// Conversion for temperature = 175.72 / 65536 (datasheet pg. 15)
		/// </summary>
		private const double TEMP_FACTOR = 0.0026812744140625;
		/// <summary>
		/// Temperature offset (datasheet pg. 15)
		/// </summary>
		private const double TEMP_OFFSET = -46.85;

		public override int Address {
			get {
				// Datasheet page 10
				return 0x40;
			}
		}
		/// <summary>
		/// Temperature follows continuity principles and should not change too rapidly. Once
		/// per second is adequate.
		/// </summary>
		public override int LogInterval {
			get {
				return 1000;
			}
		}

		// HTU sensor address cannot be configured in hardware or software
		public HTU21I2CSensor() { }
		public override string Init(I2cDevice device) {
			// Trigger a soft reset which will wipe the registers to defaults
			// This means that resolution is 12-bit RH/14-bit Temperature
			device.Write(new byte[] { 0xFE });
			return "TEMP_C,REL_HUMID";
		}
		/// <summary>
		/// Measures data from the HTU21D.
		/// </summary>
		/// <param name="device">The I2C device</param>
		/// <param name="command">MEASURE_TEMP or MEASURE_HUMIDITY</param>
		/// <returns>The measured value in quids FS</returns>
		private async Task<int> Measure(I2cDevice device, int command) {
			byte[] data = new byte[3];
			// No Hold Master mode
			device.Write(new byte[] { (byte)command });
			await Task.Delay(30);
			// Max measurement time is 50 ms for temperature and 16 ms for humidity
			for (int i = 0; i < 3; i++) {
				await Task.Delay(15);
				try {
					device.Read(data);
					// Data is MSB-first in the first 2 bytes
					int value = data[1] & 0xFC;
					return value | ((data[0] & 0xFF) << 8);
				} catch (IOException) {
					// Swallow the NACK and try again in 10 ms
				}
			}
			throw new IOException("Still measuring, increase delay!");
		}
		public override async Task<string> Sample(I2cDevice device) {
			// Measure the data we need
			double temp = (await Measure(device, MEASURE_TEMP)) * TEMP_FACTOR + TEMP_OFFSET;
			double humid = (await Measure(device, MEASURE_HUMID)) * HUMID_FACTOR +
				HUMID_OFFSET;
			return String.Format("{0:F2},{1:F2}", temp, humid);
		}
	}
}
