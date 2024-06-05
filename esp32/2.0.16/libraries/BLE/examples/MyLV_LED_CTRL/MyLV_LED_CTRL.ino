/*
    Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleWrite.cpp
    Ported to Arduino ESP32 by Evandro Copercini
*/

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define LED_CHARACTERISTIC_UUID "19B10011-E8F2-537E-4F6C-D104768A1214"
#define BTN_CHARACTERISTIC_UUID "19B10012-E8F2-537E-4F6C-D104768A1214"

// declare LED control pin 
const int ledPin = 2; // set ledPin to on-board LED 
// keep in mind that LED_BUILTIN ---> PIN 13

// declare button pin for LED control purpose
const int buttonPin = 4; // set buttonPin to digital pin 4 

void LEDController(bool isTurnON) {
	if (isTurnON) 
	{
		Serial.println("Application ---> turn the LED : ON ");
		digitalWrite(ledPin, HIGH);
	}
	else 
	{
		Serial.println("Application ---> turn the LED : OFF ");
		digitalWrite(ledPin, LOW);
	}
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string value = pCharacteristic->getValue();

      if (value.length() > 0) {
        Serial.println("*********");
        Serial.print("New value: ");
        for (int i = 0; i < value.length(); i++)
          Serial.print(value[i]);
	
		std::string state_off = "0";
		bool isTurnedON = (value != state_off);
		
		if (isTurnedON) 
		{
			Serial.println("invoke LEDController func ----------->>>>");
			LEDController(true);
		}
		
        Serial.println();
        Serial.println("*********");
      }
    }
};


void setup() {
  Serial.begin(115200);

  Serial.println("1- Download and install an BLE scanner app in your phone");
  Serial.println("2- Scan for BLE devices in the app");
  Serial.println("3- Connect to MyESP32");
  Serial.println("4- Go to CUSTOM CHARACTERISTIC in CUSTOM SERVICE and write something");
  Serial.println("5- See the magic =)");
  
  // setup LED control pin and button pin 
  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT);
 
  BLEDevice::init("MyESP32");
  BLEServer *pServer = BLEDevice::createServer();

  BLEService *pService = pServer->createService(SERVICE_UUID);

  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );

  pCharacteristic->setCallbacks(new MyCallbacks());

  pCharacteristic->setValue("Hello World");
   
  pService->start();

  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->start();
}

void loop() {

  // reading button pin value
  char buttonValue = digitalRead(buttonPin);
  
  // printout button status
  Serial.println("Button Status : ");
  Serial.println(buttonValue);
  
  
  // validate ButtonStatus pressed
  if (buttonValue == HIGH) 
  {
	  Serial.println("Switch ---> turn the LED : ON ");
	  digitalWrite(ledPin, HIGH);
	  
	  // delay for 2 seconds
	  delay(2000);
  } 
  else 
  {
	  Serial.println("Switch ---> turn the LED : OFF");
	  digitalWrite(ledPin, LOW);
  }
  
}