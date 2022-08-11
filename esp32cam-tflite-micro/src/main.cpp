#include <Arduino.h>
#include "tflm_class.h"
#include "digit.h"
#include "model_data.h"

#include "esp_camera.h"
#include "FS.h"                // SD Card ESP32
#include "SD_MMC.h"            // SD Card ESP32
#include "soc/soc.h"           // Disable brownour problems
#include "soc/rtc_cntl_reg.h"  // Disable brownour problems
#include "driver/rtc_io.h"
#include "esp32_ov2640.h"
#include "esp32_sd.h"

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define SERVICE_NAME        "TFLITE_ESP32"
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

#define in_size 240
#define out_size 60
#define downscale 4

void downsample_float(uint8_t* in, uint8_t* out)
{
    for (int i = 0; i < in_size; i+=downscale)
    {
        for (int j = 0; j < in_size; j+=downscale)
        {
            uint8_t max_r = 0;
            uint8_t max_g = 0;
            uint8_t max_b = 0;

            for (int a = 0; a < downscale; a++)
            {
                for (int b = 0; b < downscale; b++)
                {
                    int pos = i*in_size*3 + j*3 + a*in_size*3 + b*3;
                    if (in[pos] > max_r)
                        max_r = in[pos];
                    if (in[pos + 1] > max_g)
                        max_g = in[pos + 1];
                    if (in[pos + 2] > max_b)
                        max_b = in[pos + 2];
                }
            }

            int pos = i/downscale * out_size*3 + j*3/downscale;
            out[pos] = max_r;
            out[pos + 1] = max_g;
            out[pos + 2] = max_b;
        }
    }
}

TFLM_Net *nn;
camera_fb_t * fb = NULL;

BLEServer *pServer;
BLEService *pService;
BLECharacteristic inference(CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ);

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

void checkToReconnect()
{
  // disconnected so advertise
  if (!deviceConnected && oldDeviceConnected) {
    delay(500);
    pServer->startAdvertising(); // restart advertising
    oldDeviceConnected = deviceConnected;
  }
  // connected so reset boolean control
  if (deviceConnected && !oldDeviceConnected) {
    // do stuff here on connecting
    oldDeviceConnected = deviceConnected;
  }
}

uint8_t output_array[out_size * out_size * 3];

void setup()
{
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector
  if (!camera_init()) return;
  //if (!sd_init()) return;


  // Server
  BLEDevice::init(SERVICE_NAME);
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Service
  pService = pServer->createService(SERVICE_UUID);

  // Characteristic
  pService->addCharacteristic(&inference);
  inference.setValue("Hi!");

  pService->start();
  

  // Advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();

  Serial.begin(115200);
  nn = new TFLM_Net(converted_model_tflite, 1024*460);

  fb = esp_camera_fb_get();  
  esp_camera_fb_return(fb);
}

int counter = 0;
int value[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

void loop()
{
  checkToReconnect();
  fb = esp_camera_fb_get();  
  if (!fb) 
  {
    Serial.println("Camera capture failed");
  }
  else
  {
    //Serial.println(fb->len);
  }

  uint8_t* out = NULL;
  size_t out_len;
  // subtract 54
  frame2bmp(fb, &out, &out_len);
  esp_camera_fb_return(fb);

  downsample_float(out + 54, output_array);
  free(out);

  //Serial.println(out_len);

  //sd_write("/" + String(counter++) + ".bmp", output_array, out_size * out_size * 3);

  nn->load_input(output_array, out_size*out_size*3);
  nn->run();

  String bleString = String(nn->output_float()[0]) + ", " + String(nn->output_float()[1]);
  Serial.println(bleString);

  inference.setValue(bleString.c_str());

  Serial.println("===============");
}