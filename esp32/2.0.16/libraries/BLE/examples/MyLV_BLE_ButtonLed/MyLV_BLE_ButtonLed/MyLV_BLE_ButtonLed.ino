#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
 
// BLEサーバー、サービス、キャラクタリスティックのUUID
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
 
void setup() {
  Serial.begin(115200);
 
  // BLEデバイスの初期化
  BLEDevice::init("ESP32-S3 BLE Server");
 
  // BLEサーバーの作成
  BLEServer *pServer = BLEDevice::createServer();
 
  // BLEサービスの作成
  BLEService *pService = pServer->createService(SERVICE_UUID);
 
  // BLEキャラクタリスティックの作成（読み取り専用）
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ
                                       );
 
  // キャラクタリスティックの値の設定
  pCharacteristic->setValue("Hello World");
 
  // サービスの開始
  pService->start();
 
  // BLEアドバタイジングの開始
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // 電力を節約するためのアドバタイジング間隔の設定
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  Serial.println("BLEサーバーが起動しました。");
}
 
void loop() {
  // このサンプルでは、特にループ内で行うことはありません。
  delay(2000); // 無限ループの中で適度なディレイを入れる
}
