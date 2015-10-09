using System;
using System.IO;
using System.Text;
using System.Threading.Tasks;
using Windows.Devices.I2c;

namespace HABService {
	/// <summary>
	/// Interfaces to the ADXL345 accelerometer. Samples in HW at 10 Hz and polls every 1s,
	/// emptying the FIFO of all samples it contains and logging them.
	/// </summary>
	class ADXL345I2CSensor : I2CSensor {
		/// <summary>
		/// The column guide string used in the log headers.
		/// </summary>
		public const string COLS = "ONE_SAMPLE_PER_COLUMN_IN_mG";
		/// <summary>
		/// Device ID expected value (datasheet page 16).
		/// </summary>
		public const int DEVICE_ID = 0xE5;
		/// <summary>
		/// Device ID register (datasheet page 16).
		/// </summary>
		public const int REG_DEVID = 0x00;
		/// <summary>
		/// Bandwidth and rate register (datasheet page 16).
		/// </summary>
		public const int REG_BW_RATE = 0x2C;
		/// <summary>
		/// Power and sleep control register (datasheet page 16).
		/// </summary>
		public const int REG_POWER_CTL = 0x2D;
		/// <summary>
		/// Data format register (datasheet page 17).
		/// </summary>
		public const int REG_DATA_FORMAT = 0x31;
		/// <summary>
		/// Data register X little endian, others follow (datasheet page 18).
		/// </summary>
		public const int REG_DATAX0 = 0x32;
		/// <summary>
		/// FIFO mode register (datasheet page 18).
		/// </summary>
		public const int REG_FIFO_MODE = 0x38;
		/// <summary>
		/// FIFO status register (datasheet page 18).
		/// </summary>
		public const int REG_FIFO_STATUS = 0x39;
		/// <summary>
		/// The ADXL345 default slave address with the SDO pin HIGH (datasheet page 10).
		/// </summary>
		public const int SLAVE_ADDRESS = 0x1D;

		public override int Address {
			get {
				return address;
			}
		}
		/// <summary>
		/// Check every 0.5s to save power, sensor samples at 25 Hz with a 32-deep FIFO
		/// </summary>
		public override int LogInterval {
			get {
				return 500;
			}
		}

		/// <summary>
		/// The I2C address of the ADXL345.
		/// </summary>
		private readonly int address;

		/// <summary>
		/// Create a connection to the default ADXL345 address.
		/// </summary>
		public ADXL345I2CSensor() : this(SLAVE_ADDRESS) { }
		/// <summary>
		/// Creates a connection to a custom ADXL345 slave address. The valid choices
		/// according to page 10 of the datasheet are 0x1D and 0x53.
		/// <param name="address">The custom I2C address</param>
		/// </summary>
		public ADXL345I2CSensor(int address) {
			this.address = address;
		}
		public override string Init(I2cDevice device) {
			byte[] data = new byte[1];
			// Read 0x00 (DEVID)
			device.WriteRead(new byte[] { (byte)REG_DEVID }, data);
			int devID = data[0] & 0xFF;
			if (devID != DEVICE_ID)
				throw new IOException(String.Format("Device ID mismatch. Expected: 0x{0:X2}" +
					" Actual: 0x{1:X2}", DEVICE_ID, devID));
			// Full resolution mode, right justified mode, maximum range (16 G)
			device.Write(new byte[] { (byte)REG_DATA_FORMAT, 0x0B });
			// Enable streaming data mode, set watermark to 12 samples (not used!)
			device.Write(new byte[] { (byte)REG_FIFO_MODE, 0x8C });
			// Configure to regular power 25Hz mode, turn on measurement mode, disable
			// all interrupts
			device.Write(new byte[] { (byte)REG_BW_RATE, 0x08, 0x08, 0x00 });
			// Column guide
			return COLS;
		}
		public override async Task<string> Sample(I2cDevice device, object gsl) {
			StringBuilder samples = new StringBuilder(128);
			lock (gsl) {
				// Determine how much is available
				byte[] samplesB = new byte[1];
				device.WriteRead(new byte[] { (byte)REG_FIFO_STATUS }, samplesB);
				int sampleCount = samplesB[0] & 0x3F;
				if (sampleCount == 0)
					// This is not good, we need to know this
					samples.Append("NODATA");
				else {
					byte[] data = new byte[6];
					byte[] reg = new byte[] { (byte)REG_DATAX0 };
					// Read this many
					for (int i = 0; i < sampleCount; i++) {
						device.WriteRead(reg, data);
						// XL XH YL YH ZL ZH
						// +/- 16 g FS, scale factor is 3.9 mg/LSB (datasheet page 3)
						float x = BitConverter.ToInt16(data, 0) * 3.9F;
						float y = BitConverter.ToInt16(data, 2) * 3.9F;
						float z = BitConverter.ToInt16(data, 4) * 3.9F;
						samples.AppendFormat("{0:F0},{1:F0},{2:F0}", x, y, z);
						if (i < sampleCount - 1)
							samples.Append(',');
					}
				}
			}
			// Give up VS!
			await Task.Delay(0);
			return samples.ToString();
		}
	}
}
