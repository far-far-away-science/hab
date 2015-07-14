using Windows.UI.Xaml.Controls;

namespace test_app
{
    public sealed partial class MainPage : Page
    {
        public MainPage()
        {
            InitializeComponent();
            this.mainModel = new MainModel();
            this.mainModel.Initialize();
            this.devicesInfo.DataContext = this.mainModel;
        }

        private readonly MainModel mainModel;
    }
}
