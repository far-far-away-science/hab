using System;
using System.Collections.Generic;
using System.IO;
using Windows.Devices.I2c;
using Windows.System.Threading;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409

namespace RPiViewer {
	/// <summary>
	/// An empty page that can be used on its own or navigated to within a Frame.
	/// </summary>
	public sealed partial class MainPage : Page {
		public MainPage() {
			this.InitializeComponent();
			DispatcherTimer timer = new DispatcherTimer();
			timer.Interval = TimeSpan.FromMilliseconds(100.0);
			timer.Tick += GPSUpdate;
			timer.Start();
		}

		private async void GPSUpdate(object sender, object e) {
			I2cConnectionSettings settings = new I2cConnectionSettings(0x2A);
			settings.BusSpeed = I2cBusSpeed.FastMode;
			settings.SharingMode = I2cSharingMode.Exclusive;
			using (I2cDevice device = await I2cDevice.FromIdAsync("I2C1", settings)) {
				// Receive 8 bytes starting from lat register (0x04)
				byte[] data = new byte[8];
				device.WriteRead(new byte[] { 0x04 }, data);
				double lat = (double)BitConverter.ToInt32(data, 0) * 1E-6;
				double lon = (double)BitConverter.ToInt32(data, 4) * 1E-6;
				// Write to the screen
				txtLat.Text = lat.ToString("F6");
				txtLon.Text = lon.ToString("F6");
			}
		}
	}
}
