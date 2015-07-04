using System;
using System.Collections.Generic;
using System.IO;
using System.Threading.Tasks;
using Windows.ApplicationModel;
using Windows.ApplicationModel.Background;
using Windows.Devices.Enumeration;
using Windows.Devices.I2c;
using Windows.Storage;

namespace HABService {
	/// <summary>
	/// Task executed when the board boots up. Opens a log file and periodically logs sensor
	/// data to that file, according to the frequency specified in the sensor classes.
	/// </summary>
	public sealed class StartupTask : IBackgroundTask {
		/// <summary>
		/// The default I2C controller ID.
		/// </summary>
		private const string I2C_CONTROLLER = "I2C1";

		/// <summary>
		/// The global sensor lock to prevent multiple (async) sensors using the bus at once.
		/// </summary>
		private object gsl;
		/// <summary>
		/// The ID of the ACPI device matching I2C_CONTROLLER.
		/// </summary>
		private string i2cID;

		public StartupTask() {
			gsl = new object();
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
		/// Initializes all sensors.
		/// </summary>
		/// <param name="sensors">The sensors to be initialized</param>
		/// <param name="log">The log file</param>
		/// <returns>true if all sensors were brought up, or false otherwise</returns>
		private async Task<bool> InitSensors(List<I2CSensor> sensors, StreamWriter log) {
			// Bring up the sensors
			bool ok, allPass = true;
			foreach (I2CSensor sensor in sensors) {
				ok = false;
				for (int i = 0; i < 3 && !ok; i++)
					// 3 attempts
					using (I2cDevice device = await OpenI2CConnection(sensor.Address)) {
						try {
							await log.Log(sensor.Prefix, sensor.Init(device));
							ok = true;
						} catch (Exception e) {
							// Error when bringing up
							await log.Log(sensor.Prefix, "Exception when initializing: " +
								e.Message);
						}
					}
				// Uh oh!
				if (!ok) allPass = false;
			}
			return allPass;
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
		public async void Run(IBackgroundTaskInstance taskInstance) {
			// Get a deferral to stop us from quitting immediately
			List<I2CSensor> sensors = new List<I2CSensor>();
			FindI2CID(I2C_CONTROLLER);
			#region Sensor Configuration
			sensors.Add(new TelemetryI2CSensor());
			sensors.Add(new HTU21I2CSensor());
			#endregion
			BackgroundTaskDeferral deferral = taskInstance.GetDeferral();
			try {
				using (StreamWriter log = await OpenLog()) {
					// Header
					var ver = Package.Current.Id.Version;
					await log.Log(String.Format("HABService Version {0:D}.{1:D}.{2:D}.{3:D}",
						ver.Major, ver.Minor, ver.Revision, ver.Build));
					await log.Log("Log opened " + DateTime.Now.ToString());
					try {
						if (await InitSensors(sensors, log))
							await SensorWorker(sensors, log);
						else
							// TODO Should we continue here anyways!?
							await log.Log("Failed to initialize some sensors, bailing out!");
					} catch (Exception e) {
						// Log the fatal exception -- everything that makes it through at this
						// point is a bug and needs to be seen
						await log.Log("Unhandled exception: " + e.Message);
					}
				}
			} finally {
				deferral.Complete();
			}
		}
		private async Task<StreamWriter> OpenLog() {
			// Get the current time as the filename
			string date = DateTime.Now.ToString("yyyyMMdd-HHmmss");
			// If it collides, append a number to avoid trashing old logs
			StorageFile file = await DownloadsFolder.CreateFileAsync(date + ".log",
				CreationCollisionOption.GenerateUniqueName);
			return new StreamWriter(await file.OpenStreamForWriteAsync());
		}
		/// <summary>
		/// Samples one sensor. Broken out to allow the result to not be awaited...
		/// </summary>
		/// <param name="sensor">The sensor to sample</param>
		/// <param name="log">The log file</param>
		private async void SampleOneSensor(I2CSensor sensor, StreamWriter log) {
			try {
				using (I2cDevice device = await OpenI2CConnection(sensor.Address)) {
					await log.Log(sensor.Prefix, await sensor.Sample(device, gsl));
				}
			} catch (Exception e) {
				// Error!
				await log.Log(sensor.Prefix, "Exception when sampling: " + e.Message);
			}
		}
		/// <summary>
		/// Performs the work of reading and logging sensors. The sensors must have been
		/// initialized first!
		/// </summary>
		/// <param name="sensors">The sensors to be polled</param>
		/// <param name="log">The log file</param>
		private async Task SensorWorker(List<I2CSensor> sensors, StreamWriter log) {
			LinkedList<SensorSchedule> toDo = new LinkedList<SensorSchedule>();
			foreach (I2CSensor sensor in sensors)
				toDo.AddLast(new SensorSchedule(sensor));
			long now = 0L;
			// Do it!
			while (toDo.Count > 0) {
				// Get next deliverable
				SensorSchedule task = toDo.First.Value;
				toDo.RemoveFirst();
				I2CSensor sensor = task.Sensor;
				// Wait the difference
				if (task.At > now)
					await Task.Delay((int)(task.At - now));
				now = task.At;
				// Do it
				SampleOneSensor(sensor, log);
				// Reschedule for next time
				task.At += sensor.LogInterval;
				// Inject into the right place
				var ptr = toDo.First;
				while (ptr != null && ptr.Value.At < task.At)
					ptr = ptr.Next;
				if (ptr == null)
					// The list had only one element!
					toDo.AddLast(task);
				else
					toDo.AddBefore(ptr, task);
			}
		}
	}

	class SensorSchedule : IComparable<SensorSchedule> {
		/// <summary>
		/// When it needs to go next in milliseconds.
		/// </summary>
		public long At { get; set; }
		/// <summary>
		/// The I2C sensor to poll.
		/// </summary>
		public I2CSensor Sensor {
			get {
				return sensor;
			}
		}

		private I2CSensor sensor;

		public SensorSchedule(I2CSensor sensor) {
			this.sensor = sensor;
			At = 0L;
		}
		public int CompareTo(SensorSchedule other) {
			if (At > other.At)
				return 1;
			else if (At < other.At)
				return -1;
			return 0;
		}
	}

	static class ExtensionMethods {
		/// <summary>
		/// Logs a system message.
		/// </summary>
		/// <param name="log">The log file (this)</param>
		/// <param name="message">The message to log</param>
		public static async Task Log(this StreamWriter log, string message) {
			await log.Log("SYS", message);
		}
		/// <summary>
		/// Logs a sensor message.
		/// </summary>
		/// <param name="log">The log file (this)</param>
		/// <param name="prefix">The source generating the message</param>
		/// <param name="message">The message to log</param>
		public static async Task Log(this StreamWriter log, string prefix, string message) {
			await log.WriteLineAsync(prefix + "," + message);
			await log.FlushAsync();
		}
	}
}
