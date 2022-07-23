#include "sdrw.h"
#include "mpu6050.h"
#include "Wire.h"
uint64_t cardSize;

#define SDA 3
#define SCL 1

void setup() {  
  //Serial.begin(115200);
  //Serial.println("SDcard Testing....");

  // I2C MPU6050
  Wire.begin(SDA, SCL);
  // Turn on sensor
  MPU6050_write(MPU6050_RA_PWR_MGMT_1, 0x00);

  if(!SD_MMC.begin()){
      Serial.println("Card Mount Failed");
      return;
  }
  uint8_t cardType = SD_MMC.cardType();

  if(cardType == CARD_NONE){
      Serial.println("No SD_MMC card attached");
      return;
  }
  // Clear file
  writeFile(SD_MMC, "/data.txt", "");
  // appendFile(SD_MMC, "/hello.txt", "World!\n");
  // readFile(SD_MMC, "/hello.txt")
}

int16_t accel_data;
float accel_float;
void loop() {
  MPU6050_getAccelAxis(AXIS_Y, &accel_data);
  //Serial.printf("Used space: %llu B\n", SD_MMC.usedBytes());  

  accel_float = (float) accel_data / 16384.0;
  appendFile(SD_MMC, "/data.txt", (String(micros()) + ',' + String(accel_float, 3)+"\n").c_str());
  //readFile(SD_MMC, "/data.txt");
}