#include <BLEDevice.h>
#include <BLEUtils.h>

#define LOCK_CTRL_SIGN      1
#define UNLOCK_CTRL_SIGN    0
#define APPL_SRC            0
#define SUB_BOARD_SRC       1

static BLEUUID serviceUUID("4fafc201-1fb5-459e-8fcc-c5c9c331914b");
static BLEUUID charUUID("1c95d5e3-d8f7-413a-bf3d-7a2e5d7be87e");

// static variable for statement control
static bool doConnect = false;
static bool connected = false;
static bool doScan    = false;

// Define pointer for the BLE connection 
static BLEAdvertisedDevice *myDevice;
BLERemoteCharacteristic *pRemoteChar;

const int Lock_LED_PIN    = 25;
const int unlock_LED_PIN  = 26;
const int lock_SW_PIN     = 32;
const int unlock_SW_PIN   = 33;
const int connectionPIN   = 27;

// Connection Indicator Implementation
void ConnectionIndicatorImpl() {
  Serial.println("Set connection indicator ...");
  digitalWrite(connectionPIN, HIGH);
}

void disconnectionIndicatorImpl() {
  digitalWrite(connectionPIN, LOW);
}

// lock indicator implementation
void LockIndicatorIpl() {
    digitalWrite(Lock_LED_PIN, HIGH);
    delay(1000);
    digitalWrite(Lock_LED_PIN, LOW);

    // send control signal to Master
    String lockSignal = String(SUB_BOARD_SRC) + String(LOCK_CTRL_SIGN);
    pRemoteChar->writeValue(lockSignal.c_str(), lockSignal.length());
}

// Unlock indicator implementation
void UnLockIndicatorIpl() {
    digitalWrite(unlock_LED_PIN, HIGH);
    delay(1000);
    digitalWrite(unlock_LED_PIN, LOW);

    // send control unlock sign to master
    String unlockSign = String(SUB_BOARD_SRC)+ String(UNLOCK_CTRL_SIGN);
    pRemoteChar->writeValue(unlockSign.c_str(), unlockSign.length());
}

// handling for LOCK, UN-LOCK Switch pressed
void LockUnLockSWHandler() {
  int  lock_SW_Val = digitalRead(lock_SW_PIN);
  if (lock_SW_Val == LOW) {
    Serial.println("LOCK Status pin : ");
    Serial.println(lock_SW_Val);
    LockIndicatorIpl();
  }
  
  int  unlock_SW_Val = digitalRead(unlock_SW_PIN);
  if (unlock_SW_Val == LOW) {
    Serial.println("UN-LOCK Status pin : ");
    Serial.println(unlock_SW_Val);
    UnLockIndicatorIpl();
  }
}


static void notifyCallback(BLERemoteCharacteristic  *pBLERemoteCharacteristic,
                                           uint8_t  *pData,
                                           size_t   length,
                                           bool     isNotify) 
{
  if (pBLERemoteCharacteristic->getUUID().toString() == charUUID.toString()) {
   //convert received bytes to integer
   uint32_t counter = pData[0];
   for (int i = 1; i < length; i++) {
      counter = counter | (pData[i] << i*8);
   }

   // print out data
   Serial.println("Characteristic 1 (notify) from server :");
   Serial.println(counter);
  }
}

bool connectCharacteristic(BLERemoteService *pRemoteService, BLERemoteCharacteristic *pBLEremoteChar) {
    if (pBLEremoteChar == nullptr) {
       Serial.println("Failed to find characteristic");
       Serial.println(pBLEremoteChar->getUUID().toString().c_str());
       return false;
    }

    Serial.println("We found characteristic ");
    Serial.println(pBLEremoteChar->getUUID().toString().c_str());

    if (pBLEremoteChar->canNotify()) pBLEremoteChar->registerForNotify(notifyCallback);

    Serial.println("Characteristic is verified ");
    return true;
}

// Callback function that is called whenever a client is connected or disconnected
class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient *pClient) {
    
  }

  void onDisconnect(BLEClient *pClient) {
    connected = false;
    Serial.println("onDisconnect ...");
  }
};


// Function that is run whenever the server is connected
bool connectToServer() {
    Serial.println("performing connect to ");
    Serial.println(myDevice->getAddress().toString().c_str());

    BLEClient *pClient = BLEDevice::createClient();
    Serial.println(" Created client ");

    pClient->setClientCallbacks(new MyClientCallback()); //to do: define client callbacks method

   // Connect to the remove BLE Server.
   // If you pass BLEAdvertisedDevice instead of address, 
   // It will be recognized type of peer device address (public or private)
    pClient->connect(myDevice);
    Serial.println("- Connected to server");

    // Obtain a reference to the service we are after in the remote BLE Server.
    BLERemoteService *pRemoteService = pClient->getService(serviceUUID);
    if (pRemoteService == nullptr) {
        Serial.println("Failed to find our service UUID");
        Serial.println(serviceUUID.toString().c_str());
        pClient->disconnect();
        return false;
    }
    

    Serial.println("Found our service");
    connected = true;
    pRemoteChar = pRemoteService->getCharacteristic(charUUID);
    
    // verify characteristic 
    const bool isValidCharacteristic = connectCharacteristic(pRemoteService, pRemoteChar);
    if (!isValidCharacteristic) {
        connected = false;
        pClient-> disconnect();
        Serial.println("At least one characteristic UUID not found");
        return false;
    }

    // set Indicator for connection
    ConnectionIndicatorImpl();
    
    return true;
}

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
   void onResult(BLEAdvertisedDevice advertisedDevice) {
       Serial.println("BLE Advertised Device found : ");
       Serial.println(advertisedDevice.toString().c_str());

       // We have found device, let us now see if it contains the sevice we are looking for.
       if(advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID)) {
          BLEDevice::getScan()->stop();
          myDevice  = new BLEAdvertisedDevice(advertisedDevice);
          doConnect = true;
          doScan    = true;
       }
   }
};

void setup() {

   // configure GPIO for LED, SW control
   pinMode(Lock_LED_PIN,    OUTPUT);
   pinMode(unlock_LED_PIN,  OUTPUT);
   pinMode(connectionPIN,   OUTPUT);
   pinMode(lock_SW_PIN,     INPUT_PULLUP);
   pinMode(unlock_SW_PIN,   INPUT_PULLUP);

  // put your setup code here, to run once:
   Serial.begin(115200);
   Serial.println("LED control testing board purpose");
   BLEDevice::init("");

   // do Scan device 
   BLEScan *pBLEScan = BLEDevice::getScan();
   pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());  // todo : define callback and add as parameter
   pBLEScan->setInterval(1349);
   pBLEScan->setWindow(449);
   pBLEScan->setActiveScan(true);
   pBLEScan->start(5,false);

}

void loop() {
  // SW handling
  LockUnLockSWHandler();

  // if the flag "doConnect" is true then we have scanned for and found the desired 
  // BLEServer with which we wish to connect. Now we connect to it. Once we are connected 
  // we set the connected flag to true.
  if (doConnect == true) {
    if (connectToServer()) {
        Serial.println("We are now connected to the BLE Server");
    }
    else {
        Serial.println("We have failed to connect to the BLE Server, there is nothing more we will do");
    }
    doConnect = false;
  }

  // If we are connected to a peer BLE Server, update the characteristic each time we are reached
  // with the current time since boot.
  if (connected){
    std::string rxValue  = pRemoteChar->readValue();
    Serial.print("Characteristic (read value) : ");
    Serial.println(rxValue.c_str());

    String txValue = "String with random value from client : " + String(-random(1000));
    Serial.println("Characteristic (write value) : " + txValue);

    // Set the characteristic's value to be the array of bytes that is actually a string.
    pRemoteChar->writeValue(txValue.c_str(), txValue.length());
  }
  else if(doScan) {
    // set Indicator for DISConnection
    disconnectionIndicatorImpl();
    Serial.println("Disonnection --> perform re-Scan ");
    BLEDevice::getScan()->start(0);
  }

  // In this example "delay" is used to delay with one second. This is of course a very basic
  // Implementation to keep things simple. I recommend to use millis() for any production code
  //delay(1000);
  delay(100);

}