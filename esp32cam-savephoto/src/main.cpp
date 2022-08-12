#include <Arduino.h>

#include "esp_camera.h"
#include "FS.h"                // SD Card ESP32
#include "SD_MMC.h"            // SD Card ESP32
#include "soc/soc.h"           // Disable brownour problems
#include "soc/rtc_cntl_reg.h"  // Disable brownour problems
#include "driver/rtc_io.h"
#include "esp32_ov2640.h"
#include "esp32_sd.h"

camera_fb_t * fb = NULL;
int counter = -3;

void setup()
{
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector
  if (!camera_init()) return;
  if (!sd_init()) return;

  //pinMode(2, OUTPUT);

  Serial.begin(115200);

  fb = esp_camera_fb_get();  
  esp_camera_fb_return(fb);
}

void loop()
{
  fb = esp_camera_fb_get();  
  if (!fb) 
  {
    Serial.println("Camera capture failed");
  }
  else
  {
    //Serial.println(fb->len);
  }

  // subtract 54
  /*
  uint8_t* out = NULL;
  size_t out_len;
  frame2bmp(fb, &out, &out_len);
  */
  if (counter >= 110)
    while(1);

  Serial.println("Hey");
  if (counter++ >= 0)
    sd_write("/" + String(counter) + ".jpg", fb->buf, fb->len);
  esp_camera_fb_return(fb);
  delay(1000);
}