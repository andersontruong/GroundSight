#include <Arduino.h>
#include <Wire.h>
#include "mpu6050.h"
#include "sdrw.h"
#include "imu_filter.h"

#include "esp_camera.h"
#include "FS.h"                // SD Card ESP32
#include "SD_MMC.h"            // SD Card ESP32
#include "soc/soc.h"           // Disable brownour problems
#include "soc/rtc_cntl_reg.h"  // Disable brownour problems
#include "driver/rtc_io.h"
#include <EEPROM.h>            // read and write from flash memory

#define EEPROM_SIZE 1

// Pin definition for CAMERA_MODEL_AI_THINKER
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

int pictureNumber = 0;

camera_fb_t * fb = NULL;

/* ===== MPU-6050 I2C ===== */
#define SDA 3
#define SCL 1
/* ===== END MPU-6050 I2C ===== */


/* ===== DSP ===== */
#define X_BUFFER_SIZE 5
#define Y_BUFFER_SIZE 4

float b_coeff[X_BUFFER_SIZE] = {9.500782e-05, 3.800313e-04, 5.700469e-04, 3.800313e-04, 9.500782e-05};
float a_coeff[Y_BUFFER_SIZE] = {-3.449698, 4.495498, -2.620084, 0.575804};

imu_buffer_t* inputs;
imu_buffer_t* outputs;

// Anteroposterior acceleration
int16_t ap_accel = 0;
// Filtered signal
int16_t filtered_accel = 0;
int16_t prev_filtered_accel = 0;

bool dataAvailable = false;

// Difference
int16_t diff_accel = 0;
int16_t prev_diff_accel = 0;
/* ===== END DSP ===== */

int counter = 0;

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG; 
  
  if(psramFound()){
    config.frame_size = FRAMESIZE_UXGA; // FRAMESIZE_ + QVGA|CIF|VGA|SVGA|XGA|SXGA|UXGA
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }
  
  // Init Camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    return;
  }
  
  //Serial.println("Starting SD Card");
  if(!SD_MMC.begin()){
    return;
  }
  
  uint8_t cardType = SD_MMC.cardType();
  if(cardType == CARD_NONE){
    return;
  }




  
  // MPU-6050 I2C
  Wire.begin(SDA, SCL);
  MPU6050_write(MPU6050_RA_PWR_MGMT_1, 0x00);

  pinMode(4, INPUT);
  digitalWrite(4, LOW);
  rtc_gpio_hold_dis(GPIO_NUM_4);

/*
  // Setup SD card
  if(!SD_MMC.begin())
  {
    while (1);
  }
  uint8_t cardType = SD_MMC.cardType();

  if(cardType == CARD_NONE){
    while (1);
  }

  // Empty file
  writeFile(SD_MMC, "/raw.txt", "");
  writeFile(SD_MMC, "/filtered.txt", "");
  writeFile(SD_MMC, "/trigger.txt", "");
  */

  inputs = imu_buffer_init(Y_BUFFER_SIZE);
  outputs = imu_buffer_init(X_BUFFER_SIZE);
}

void loop() 
{
  MPU6050_getAccelAxis(AXIS_Y, &ap_accel);
  filtered_accel = imu_filter(ap_accel, inputs, b_coeff, outputs, a_coeff);
  diff_accel = filtered_accel - prev_filtered_accel;

  //appendFile(SD_MMC, "/raw.txt", (String(micros()) + ',' + String(ap_accel) + "\n").c_str());
  //appendFile(SD_MMC, "/filtered.txt", (String(micros()) + ',' + String(filtered_accel) + "\n").c_str());
  //appendFile(SD_MMC, "/diff.txt", (String(micros()) + ',' + String(diff_accel) + "\n").c_str());

  if (diff_accel > 50 && prev_diff_accel < 0) //(diff_accel >= 0 && prev_diff_accel < 0)
  {
    //digitalWrite(4, HIGH);
    //counter = 100;
    fb = esp_camera_fb_get();  
    if(!fb) 
    {
      Serial.println("Camera capture failed");
      return;
    }
    // initialize EEPROM with predefined size
    EEPROM.begin(EEPROM_SIZE);
    pictureNumber = EEPROM.read(0) + 1;

    // Path where new picture will be saved in SD Card
    String path = "/picture" + String(pictureNumber) +".jpg";

    fs::FS &fs = SD_MMC; 
    Serial.printf("Picture file name: %s\n", path.c_str());
    
    File file = fs.open(path.c_str(), FILE_WRITE);
    file.write(fb->buf, fb->len); // payload (image), payload length
    EEPROM.write(0, pictureNumber);
    EEPROM.commit();
    file.close();
    esp_camera_fb_return(fb); 
  }
  /*
  if (--counter == 0)
  {
    digitalWrite(4, LOW);
  }
  */
  

  prev_filtered_accel = filtered_accel;
  prev_diff_accel = diff_accel;
}