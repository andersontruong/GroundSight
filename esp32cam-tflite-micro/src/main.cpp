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

TFLM_Net *nn;
camera_fb_t * fb = NULL;

void setup()
{
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector
  if (!camera_init()) return;
  if (!sd_init()) return;

  Serial.begin(115200);
  nn = new TFLM_Net(converted_model_tflite, 1028*60);

  fb = esp_camera_fb_get();  
  esp_camera_fb_return(fb);
}

int counter = 0;
float downscale[784];
int value[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

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

  uint8_t* out = NULL;
  size_t out_len;
  // subtract 54
  frame2bmp(fb, &out, &out_len);
  Serial.println(out_len);
  free(out);

  for (int i = 54; i < out_len; i++)
  {
    Serial.print(out[i]);
    Serial.print(", ");
  }

  Serial.println();
    

/*
  int scale = 4;
  for (int row = 0; row < 96; row += scale)
    for (int col = 0; col < 96; col += scale)
    {
      if (fb->buf[col + row*96] > 95)
        downscale[(int)(floor(col/scale) + floor(row*28/scale))] = 0.0;
      else
        downscale[(int)(floor(col/scale) + floor(row*28/scale))] = (255 - fb->buf[col + row*96]) / 255.0;
        
    }
  nn->load_input(downscale, 784);
  nn->run();

  float max = 0;
  int max_index = 0;

  for (size_t i = 0; i < 10; i++)
  {
    Serial.print(nn->output_float()[i]);
    Serial.print(" ");
    if (nn->output_float()[i] > max)
    {
      max_index = i;
      max = nn->output_float()[i];
    }
  }
  Serial.println(value[max_index]);
  Serial.println('\n');
*/
  //sd_write("/" + String(counter++) + ".bmp", bmp, fb->len * 3);

  esp_camera_fb_return(fb);

  delay(500);
  /*
  nn->load_input(digit, 784);
  nn->run();

  for (size_t i = 0; i < 10; i++)
  {
    Serial.print(nn->output_int8()[i]);
    Serial.print(", ");
  }
  Serial.println('\n');
  */

  /*
  float number1 = random(100) / 100.0;
  float number2 = random(100) / 100.0;

  nn->getInputBuffer()[0] = number1;
  nn->getInputBuffer()[1] = number2;

  float* result = nn->out();

  const char *expected = number2 > number1 ? "True" : "False";

  const char *predicted = result > 0.5 ? "True" : "False";

  Serial.printf("%.2f %.2f - result %.2f - Expected %s, Predicted %s\n", number1, number2, result, expected, predicted);
*/
}