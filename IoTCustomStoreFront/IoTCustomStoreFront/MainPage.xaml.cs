using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.Services.Store;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409

namespace IoTCustomStoreFront
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class MainPage : Page
    {
        private StoreContext _storeContext = null;


        private void InitStoreContext()
        {
            _storeContext = StoreContext.GetDefault();
        }

        public MainPage()
        {
            this.InitializeComponent();

            GetAddons();
        }

        public async void GetAddons()
        {
            if (_storeContext == null)
                InitStoreContext();

            // Specify the kinds of add-ons to retrieve.
            string[] productKinds = { "Durable", "Consumable", "UnmanagedConsumable" };
            List<String> filterList = new List<string>(productKinds);

            StoreProductQueryResult queryResult = await _storeContext.GetAssociatedStoreProductsAsync(filterList);

            if (queryResult.ExtendedError != null)
            {
                // The user may be offline or there might be some other server failure.
                Debug.WriteLine(queryResult.ExtendedError.Message);
                return;
            }

            foreach (KeyValuePair<string, StoreProduct> item in queryResult.Products)
            {
                // Access the Store product info for the add-on.
                StoreProduct product = item.Value;
                Debug.WriteLine(product);
                // Use members of the product object to access listing info for the add-on...
            }
        }
    }
}
