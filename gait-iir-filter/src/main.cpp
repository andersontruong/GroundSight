#include <Arduino.h>
#include <Wire.h>
#include "mpu6050.h"
#include "sdrw.h"
#include "imu_filter.h"
#include "Adafruit_NeoPixel.h"
#ifdef __AVR__
  #include <avr/power.h>
#endif

/* ===== NEOPIXEL LED ===== */
#define LED_PIN 12

Adafruit_NeoPixel led(1, LED_PIN, NEO_GRB + NEO_KHZ800);
/* ===== END NEOPIXEL LED ===== */



/* ===== MPU-6050 I2C ===== */
#define SDA 3
#define SCL 1
/* ===== END MPU-6050 I2C ===== */



/* ===== DSP ===== */
#define X_BUFFER_SIZE 5
#define Y_BUFFER_SIZE 4

float b_coeff[X_BUFFER_SIZE] = {5.451273e-06, 2.180509e-05, 3.270763e-05, 2.180509e-05,
                    5.451273e-06};
float a_coeff[Y_BUFFER_SIZE] = {-3.739369, 5.251535, -3.282506, 0.770427};

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



/* ===== PARALLEL TASK ===== */

// Accelerometer reading and DSP on Core 1
// SD Writing on Core 0

TaskHandle_t TaskCore0;
/* ===== END PARALLEL TASK ===== */



void SDWrite( void * pvParameters );

void setup() {
  // MPU-6050 I2C
  Wire.begin(SDA, SCL);
  MPU6050_write(MPU6050_RA_PWR_MGMT_1, 0x00);

  led.begin();
  led.clear();

  // Setup SD card
  SD_MMC.begin();

  // Empty file
  writeFile(SD_MMC, "/raw.txt", "");
  writeFile(SD_MMC, "/filtered.txt", "");

  inputs = imu_buffer_init(Y_BUFFER_SIZE);
  outputs = imu_buffer_init(X_BUFFER_SIZE);

  // Begin task on core 0
  int16_t* params[2] = {&filtered_accel, &prev_diff_accel};

  xTaskCreatePinnedToCore(
    SDWrite, // Function to implement the task
    "TaskCore0",   // Name of the task 
    10000,     // Stack size in words 
    (void*)&filtered_accel,      // Task input parameter 
    1,         // Priority of the task 
    &TaskCore0,    // Task handle.
    0);        // Core where the task should run
}

void SDWrite( void* params ){
  //Serial.println(xPortGetCoreID());

  for (;;)
  {
    //appendFile(SD_MMC, "/raw.txt", (String(micros()) + ',' + String(ap_accel) + '\n').c_str());
    //appendFile(SD_MMC, "/filtered.txt", (String(micros()) + ',' + String(filtered_accel) + '\n').c_str());
    //dataAvailable = false;

    if (*((int16_t*)params) >= 1000)
    {
      led.setPixelColor(0, led.Color(200, 0, 0));
      led.show();
      //led.show();
    }
    else
    {
      led.setPixelColor(0, led.Color(0, 0, 0));
      led.show();
    }
  }
}

void loop() 
{
  MPU6050_getAccelAxis(AXIS_Y, &ap_accel);
  filtered_accel = imu_filter(ap_accel, inputs, b_coeff, outputs, a_coeff);
  dataAvailable = true;
  diff_accel = filtered_accel - prev_filtered_accel;
  delay(1);
  prev_filtered_accel = filtered_accel;
  prev_diff_accel = diff_accel;
}