using System.Threading.Tasks;
using Windows.Devices.I2c;

namespace HABService {
	/// <summary>
	/// A class which parents all I2C sensors connected to the Pi. Specifies the sensor
	/// logging conditions and provides a method to log the desired data.
	/// </summary>
	abstract class I2CSensor {
		/// <summary>
		/// Reports the I2C address (7-bit right aligned) of the sensor to be sampled. This
		/// value must be constant, but it can be set in the constructor to allow multiple
		/// devices with the same driver to coexist.
		/// </summary>
		public abstract int Address { get; }
		/// <summary>
		/// Report the interval, in milliseconds, that this sensor should be logged. This
		/// value may be dynamic; it is checked again on each reschedule.
		/// </summary>
		public abstract int LogInterval { get; }
		/// <summary>
		/// Returns the prefix to be used when logging messages from this source.
		/// </summary>
		public string Prefix {
			get {
				// The log prefix, always 3 letters/digits
				return GetType().Name.PadRight(3).Substring(0, 3).ToUpper();
			}
		}

		/// <summary>
		/// Initializes this I2C sensor. If this method fails, then it will be retried up to
		/// two more times (3 times in total) before the background task will give up and
		/// stop attempting to read this device.
		/// </summary>
		/// <param name="device">The device to initialize</param>
		/// <returns>A message line which will be logged</returns>
		/// <exception cref="IOException">If an error occurs when initializing the
		/// sensor</exception>
		public abstract string Init(I2cDevice device);
		/// <summary>
		/// Checks the sensor and reports a message to be logged.
		/// </summary>
		/// <param name="device">A device instance opened to the address specified by this
		/// class</param>
		/// <param name="gsl">The Global Sensor Lock which must be taken during I2C
		/// communications</param>
		/// <returns>The data read from this sensor, to be inserted into the log
		/// file as a record</returns>
		/// <exception cref="IOException">If an error occurs when connecting to the
		/// sensor</exception>
		public abstract Task<string> Sample(I2cDevice device, object gsl);
	}
}
