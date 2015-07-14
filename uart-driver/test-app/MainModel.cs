using System;
using System.ComponentModel;

using Windows.Devices.Enumeration;
using Windows.Devices.SerialCommunication;

namespace test_app
{
    class MainModel : INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler PropertyChanged = delegate { };

        public async void Initialize()
        {
            string serialQuery = SerialDevice.GetDeviceSelector();
            DeviceInformationCollection devicesCollection = await DeviceInformation.FindAllAsync(serialQuery);
            int numberOfDevices = 0;
            foreach (var deviceInfo in devicesCollection)
            {
                ++numberOfDevices;
            }
            NumberOfDevices = numberOfDevices;
        }

        public int NumberOfDevices
        {
            get { return this.numberOfDevices; }
            private set
            {
                if (this.numberOfDevices != value)
                {
                    this.numberOfDevices = value;
                }
                PropertyChanged(this, new PropertyChangedEventArgs("NumberOfDevices"));
            }
        }

        private int numberOfDevices;
    }
}
