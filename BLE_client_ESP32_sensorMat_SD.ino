/**
   A BLE client example that is rich in capabilities.
   There is a lot new capabilities implemented.
   author unknown
   updated by chegewara
*/

#include "BLEDevice.h"
//#include "BLEScan.h"
#include <SPI.h>
#include <SD.h>

const int chipSelect = 14;
uint16_t manufac[6] = {0xCC, 0xCC, 0xCC, 0x0A, 0x39, 0x8F};
uint8_t oldD[13];
uint8_t newD[13];

// The remote service we wish to connect to.
static BLEUUID serviceUUID("000055C0-0000-1000-8000-00802EC611CD");

static BLEUUID serviceUUIDConnect("000055C0-0000-1000-8000-00805F9B34FB");
// The characteristic of the remote service we are interested in.
static BLEUUID    charUUID("000055C2-0000-1000-8000-00805F9B34FB");

static boolean doConnect = false;
static boolean connected = false;
static boolean doScan = false;
static BLERemoteCharacteristic* pRemoteCharacteristic;
static BLEAdvertisedDevice* myDevice;

uint8_t sensorvalues[13];

static void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {
  Serial.print("Notify callback for characteristic ");
  Serial.print(pBLERemoteCharacteristic->getUUID().toString().c_str());
  Serial.print(" of data length ");
  Serial.println(length);
  Serial.print("data: ");
  //int arrayDATA[]=(char*)pData
  //uint8_t test[]=&pData;
  //Serial.println((char*)pData);
  //String dataa=(char*)pData;

  //uint8_t* pData.toCharArray(char dataa,13);

  //toCharArray((char*)pData,13);

  //    byte * i;
  //    for (i=(byte*)pData; *i; i++) {
  //}
  for (int c = 0; c < 13; c++) {
    //Serial.println(*(pData+c));
    sensorvalues[c] = *(pData + c);
    Serial.print(sensorvalues[c]);
    Serial.print(',');
    newD[c] = sensorvalues[c];
  }
  Serial.println(' ');
  //Serial.println(test[0]);

}

class MyClientCallback : public BLEClientCallbacks {
    void onConnect(BLEClient* pclient) {
    }

    void onDisconnect(BLEClient* pclient) {
      connected = false;
      Serial.println("onDisconnect");
    }
};

bool connectToServer() {
  Serial.print("Forming a connection to ");
  Serial.println(myDevice->getAddress().toString().c_str());

  BLEClient*  pClient  = BLEDevice::createClient();
  Serial.println(" - Created client");

  pClient->setClientCallbacks(new MyClientCallback());

  // Connect to the remove BLE Server.
  pClient->connect(myDevice);  // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
  Serial.println(" - Connected to server");

  // Obtain a reference to the service we are after in the remote BLE server.
  BLERemoteService* pRemoteService = pClient->getService(serviceUUIDConnect);
  if (pRemoteService == nullptr) {
    Serial.print("Failed to find our service UUID: ");
    Serial.println(serviceUUID.toString().c_str());
    pClient->disconnect();
    return false;
  }
  Serial.println(" - Found our service");


  // Obtain a reference to the characteristic in the service of the remote BLE server.
  pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
  if (pRemoteCharacteristic == nullptr) {
    Serial.print("Failed to find our characteristic UUID: ");
    Serial.println(charUUID.toString().c_str());
    pClient->disconnect();
    return false;
  }
  Serial.println(" - Found our characteristic");

  // Read the value of the characteristic.
  if (pRemoteCharacteristic->canRead()) {
    std::string value = pRemoteCharacteristic->readValue();
    Serial.print("The characteristic value was: ");
    Serial.println(value.c_str());
  }

  if (pRemoteCharacteristic->canNotify())
    pRemoteCharacteristic->registerForNotify(notifyCallback);

  connected = true;
  return true;
}
/**
   Scan for BLE servers and find the first one that advertises the service we are looking for.
*/
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    /**
        Called for each advertising BLE server.
    */
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      Serial.print("BLE Advertised Device found: ");
      Serial.println(advertisedDevice.toString().c_str());

      // We have found a device, let us now see if it contains the service we are looking for.
      if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID)) {

        BLEDevice::getScan()->stop();
        myDevice = new BLEAdvertisedDevice(advertisedDevice);
        doConnect = true;
        doScan = true;

      } // Found our server
    } // onResult
}; // MyAdvertisedDeviceCallbacks


void setup() {
  Serial.begin(115200);
  Serial.println("Starting Arduino BLE Client application...");
  BLEDevice::init("");

  // Retrieve a Scanner and set the callback we want to use to be informed when we
  // have detected a new device.  Specify that we want active scanning and start the
  // scan to run for 5 seconds.
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5, false);

  Serial.print("Initializing SD card...");
  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    while (1);
  }
  Serial.println("card initialized.");

} // End of setup.


// This is the Arduino main loop function.
void loop() {

  // If the flag "doConnect" is true then we have scanned for and found the desired
  // BLE Server with which we wish to connect.  Now we connect to it.  Once we are
  // connected we set the connected flag to be true.
  if (doConnect == true) {
    if (connectToServer()) {
      Serial.println("We are now connected to the BLE Server.");
    } else {
      Serial.println("We have failed to connect to the server; there is nothin more we will do.");
    }
    doConnect = false;
  }

  // If we are connected to a peer BLE Server, update the characteristic each time we are reached
  // with the current time since boot.
  if (connected) {
    String newValue = "Time since boot: " + String(millis() / 1000);
    //Serial.println("Setting new characteristic value to \"" + newValue + "\"");

    // Set the characteristic's value to be the array of bytes that is actually a string.
    pRemoteCharacteristic->writeValue(newValue.c_str(), newValue.length());

  } else if (doScan) {
    BLEDevice::getScan()->start(0);  // this is just example to start scan after disconnect, most likely there is better way to do it in arduino
  }


  if (oldD[12] != newD[12]) {
    //memcpy(newD,oldD,sizeof(newD));
    //Serial.println("Here1");
    oldD[12] = newD[12];
    String dataString = "";
    for (int i = 0; i < 13; i++) {
      dataString += String(newD[i]);
      dataString += " ";
    }
    //Serial.println(dataString);
    File dataFile = SD.open("/datalog.txt", FILE_APPEND);
    if (dataFile) {
      //Serial.println("Here2");
      dataFile.println(dataString);
      Serial.println(dataString);
      dataFile.close();
      // print to the serial port too:
      
    }
    else {
      Serial.println("error opening datalog.txt");
    }
  }

  delay(10); // Delay 1/10 second between loops.
} // End of loop
