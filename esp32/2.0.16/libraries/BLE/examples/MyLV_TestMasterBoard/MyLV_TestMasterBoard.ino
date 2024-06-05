/*
  Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleNotify.cpp
  Ported to Arduino ESP32 by Evandro Copercini
  updated by chegewara and MoThunderz
*/
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/
// UUIDs used in this example:
#define SERVICE_UUID          "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID   "1c95d5e3-d8f7-413a-bf3d-7a2e5d7be87e"

#define CONNECTION_MAX        2

// Initialize all pointers
BLEServer *pServer = NULL;
BLECharacteristic *pCharacteristic = NULL;
BLE2902 *pBLE2902;

// Some variables to keep track on device connected
bool deviceConnected      = false;
bool oldDeviceConnected   = false;

uint8_t connectedDeviceCounter = 0;

// define Lock, Unlock control PIN
const int Lock_LED_PIN    = 18;
const int unlock_LED_PIN  = 19;
const int lock_SW_PIN     = 4;
const int unlock_SW_PIN   = 23;

// control LED lock 
void indicatorLockHandler() {
    digitalWrite(Lock_LED_PIN, HIGH);
    delay(1000);
    digitalWrite(Lock_LED_PIN, LOW);
}

// control LED unlock 
void indicatorUnLockHandler() {
    digitalWrite(unlock_LED_PIN, HIGH);
    delay(1000);
    digitalWrite(unlock_LED_PIN, LOW);
};

// handling for LOCK, UN-LOCK Switch pressed
void LockUnLockSWHandler() {
  int  lock_SW_Val = digitalRead(lock_SW_PIN);
  if (lock_SW_Val == LOW) {
    Serial.println("LOCK Status pin : ");
    Serial.println(lock_SW_Val);
    indicatorLockHandler();
  }
  
  int  unlock_SW_Val = digitalRead(unlock_SW_PIN);
  if (unlock_SW_Val == LOW) {
    Serial.println("UN-LOCK Status pin : ");
    Serial.println(unlock_SW_Val);
    indicatorUnLockHandler();
  }
}


// Callback function that is called whenever a client is connected or disconnected
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      connectedDeviceCounter++; 
      Serial.print("onConnect --> Connected devices counter : ");
      Serial.println(connectedDeviceCounter);
      if (connectedDeviceCounter < CONNECTION_MAX) {
         //  advertising --> enable seconds device connection
         pServer->startAdvertising(); 
         Serial.println("start advertising for next connection");
      }      
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      Serial.print("onDisconnect --> Connected devices counter : ");
      Serial.println(connectedDeviceCounter);
      connectedDeviceCounter--;
    }
};

class CharacteristicCallBack: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pChar) {
    std::string remoteValue = pChar->getValue();
    String showValue = String(remoteValue.c_str(), remoteValue.length());
    Serial.println(showValue);

    const uint8_t applicationID  = '0';
    const uint8_t subBoardID     = '1';
    const uint8_t lockSignalID   = '1';
    const uint8_t unlockSignalID = '0';

    if ((subBoardID == remoteValue[0]) || (applicationID == remoteValue[0])) {
        if (lockSignalID   == remoteValue[1]) {
            Serial.println("LOCK REQUEST ...");
            indicatorLockHandler();
        }
        else if (unlockSignalID == remoteValue[1]) {
            Serial.println("UN-LOCK REQUEST ...");
            indicatorUnLockHandler();
        }
        else {
            // do nothing
        }
    }
  }
};

void setup() {
  Serial.begin(115200);

  // configure GPIO for SW and Indicator's Lock & Unlock
  pinMode(Lock_LED_PIN,    OUTPUT);
  pinMode(unlock_LED_PIN,  OUTPUT);
  pinMode(lock_SW_PIN,     INPUT_PULLUP);
  pinMode(unlock_SW_PIN,   INPUT_PULLUP);  

  // Enable Watchdog prevent reset for core 1
  //enableCore1WDT();

  // Create the BLE Device
  BLEDevice::init("ESP32");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);  

  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY
                    ); 

  // Creating descriptor and attach to characteristic
  pBLE2902 = new BLE2902();
  pBLE2902->setNotifications(true);
  pCharacteristic->addDescriptor(pBLE2902);

  // Add callback functions here:
  pCharacteristic->setCallbacks(new CharacteristicCallBack());
  
  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
  BLEDevice::startAdvertising();
  Serial.println("Waiting a client connection to notify...");
}

void loop() {
    // SW handling
    LockUnLockSWHandler();

    // The code below keeps the connection status uptodate:
    // Disconnecting
    if (!deviceConnected && oldDeviceConnected) {
        delay(500); // give the bluetooth stack the chance to get things ready
        pServer->startAdvertising(); // restart advertising
        Serial.println("start advertising");
        oldDeviceConnected = deviceConnected;
    }
    // Connecting
    if (deviceConnected && !oldDeviceConnected) {
        // do stuff here on connecting
        oldDeviceConnected = deviceConnected;
    }
}