#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include "soc/soc.h"          // Disable brownour problems
#include "soc/rtc_cntl_reg.h" // Disable brownour problems
#include <Wire.h>
#include <string>

#include "mpu6050.h"

#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define X_ACCEL_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define Y_ACCEL_UUID "02f38517-3c2e-45b3-a7dd-ac426a2b37d6"
#define Z_ACCEL_UUID "4de8baa7-4555-48ea-a9f8-8184c023b7fd"
#define ACCEL_MAG_UUID "1ce24875-932e-4049-b99f-948746f375e4"

#define SDA 3
#define SCL 1

BLEServer *pServer = NULL;//added
BLECharacteristic* ACCEL_CHARS[4];

bool deviceConnected = false;
bool oldDeviceConnected = false;

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };
    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

void checkToReconnect() //added
{
  // disconnected so advertise
  if (!deviceConnected && oldDeviceConnected) {
    delay(500); // give the bluetooth stack the chance to get things ready
    pServer->startAdvertising(); // restart advertising
    oldDeviceConnected = deviceConnected;
  }
  // connected so reset boolean control
  if (deviceConnected && !oldDeviceConnected) {
    // do stuff here on connecting
    oldDeviceConnected = deviceConnected;
  }
}

void setup() {
  // Serial.begin(115200);
  Wire.begin(SDA, SCL);
  BLEDevice::init("GroundSight");

  // Create server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create characteristic (READ and WRITE)
  ACCEL_CHARS[0] = pService->createCharacteristic(
    X_ACCEL_UUID,
    BLECharacteristic::PROPERTY_READ
  );

  ACCEL_CHARS[1] = pService->createCharacteristic(
    Y_ACCEL_UUID,
    BLECharacteristic::PROPERTY_READ
  );     

  ACCEL_CHARS[2] = pService->createCharacteristic(
    Z_ACCEL_UUID,
    BLECharacteristic::PROPERTY_READ
  );       

  ACCEL_CHARS[3] = pService->createCharacteristic(
    ACCEL_MAG_UUID,
    BLECharacteristic::PROPERTY_READ
  );            

  // Start service
  pService->start();

  // Start advertising service
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();

  std::string id = BLEDevice::getAddress().toString();
  //Serial.println(id.c_str());
  MPU6050_write(MPU6050_RA_PWR_MGMT_1, 0x00);
}

int16_t accel_data[3];
float accel_float[3];
float accel_mag;
void loop() {
  checkToReconnect();

  //MPU6050_read(MPU6050_RA_ACCEL_XOUT_L, &data, 1);

  accel_mag = 0;
  MPU6050_getAccel(accel_data);
  for (uint8_t i = 0; i < 3; i++)
  {
    accel_float[i] = (float) accel_data[i] / 16384.0;
    accel_mag += pow(accel_float[i], 2);
    ACCEL_CHARS[i]->setValue(accel_float[i]);
  }

  accel_mag = sqrt(accel_mag);
  ACCEL_CHARS[3]->setValue(accel_mag);

  delay(10);
}