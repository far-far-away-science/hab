using System;
using System.Threading.Tasks;
using Windows.Devices.I2c;

namespace HABService {
	class TMP102I2CSensor : I2CSensor {
		/// <summary>
		/// Pointer register value for config register (datasheet pg. 15)
		/// </summary>
		public const int REG_CONFIGURATION = 0x01;
		/// <summary>
		/// Pointer register value for temperature register (datasheet pg. 15)
		/// </summary>
		public const int REG_TEMPERATURE = 0x00;
		/// <summary>
		/// The TMP102 default slave address (datasheet pg. 10).
		/// </summary>
		public const int SLAVE_ADDRESS = 0x48;
		/// <summary>
		/// Conversion for temperature = 0.0625 degC / LSB (datasheet pg. 7)
		/// </summary>
		private const double TEMP_FACTOR = 0.0625;

		/// <summary>
		/// The I2C address of the TMP102.
		/// </summary>
		private readonly int address;

		public override int Address {
			get {
				return address;
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

		/// <summary>
		/// Creates a connection to the default TMP102 slave address (ADD0 = 0). This is the
		/// setting used by default on the SparkFun breakout board.
		/// </summary>
		public TMP102I2CSensor() : this(SLAVE_ADDRESS) { }
		/// <summary>
		/// Creates a connection to a custom TMP102 slave address. The valid choices according
		/// to page 10 of the datasheet are 0x48, 0x49, 0x4A, and 0x4B.
		/// </summary>
		/// <param name="address">The custom I2C address</param>
		/// <returns></returns>
		public TMP102I2CSensor(int address) {
			this.address = address;
		}
		public override string Init(I2cDevice device) {
			// The POR mode of the device is continuous conversion 4 Hz. We are OK with this
			// since "shutdown" mode with single conversion saves a whopping 9 uA (joy!)
			// Therefore, this command only ensures that the device is present
			device.Write(new byte[] { REG_CONFIGURATION, 0x60, 0xA0 });
			return "TEMP_C";
		}
		public override async Task<string> Sample(I2cDevice device, object gsl) {
			byte[] data = new byte[2];
			lock (gsl) {
				device.WriteRead(new byte[] { REG_TEMPERATURE }, data);
			}
			// Temperature is big-endian and twos complement!
			int tempRaw = ((data[0] & 0xFF) << 4) | ((data[1] & 0xF0) >> 4);
			if ((tempRaw & 0x800) != 0)
				// Sign extend bit 12
				tempRaw = -(-tempRaw & 0x7FF);
			// Give up VS!
			await Task.Delay(0);
			return String.Format("{0:F2}", tempRaw * TEMP_FACTOR);
		}
	}
}
