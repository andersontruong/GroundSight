#include "esp_camera.h"
#include "esp_timer.h"
#include "img_converters.h"
#include "Arduino.h"
#include "soc/soc.h"          // Disable brownour problems
#include "soc/rtc_cntl_reg.h" // Disable brownour problems
#include "driver/rtc_io.h"
#include "WiFi.h"
#include <WebServer.h>
#include "index.h"
#include <WiFiClient.h>

// OV2640 camera module pins (CAMERA_MODEL_AI_THINKER)
#define PWDN_GPIO_NUM 32
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 0
#define SIOD_GPIO_NUM 26
#define SIOC_GPIO_NUM 27
#define Y9_GPIO_NUM 35
#define Y8_GPIO_NUM 34
#define Y7_GPIO_NUM 39
#define Y6_GPIO_NUM 36
#define Y5_GPIO_NUM 21
#define Y4_GPIO_NUM 19
#define Y3_GPIO_NUM 18
#define Y2_GPIO_NUM 5
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM 23
#define PCLK_GPIO_NUM 22
#define FLASH_GPIO_NUM 4

// ledPin refers to ESP32-CAM GPIO 4 (flashlight)
#define FLASH_GPIO_NUM 4

// Photo File Name to save in SPIFFS
#define FILE_PHOTO "/photo.jpg"

const char* ssid = "err404_guest_not_found";
const char* password = "GoBruinsTrackandField!";

WebServer server(80);

void handleRoot() {
  String s = MAIN_page; //Read HTML contents
  server.send(200, "text/html", s); //Send web page
}
 
void handleADC() {
  static int val = 0;
  val++;
  if (val > 10)
    val = 0;
  server.send(200, "text/plane", String(val)); //Send ADC value only to client ajax request
}

void setup()
{
  Serial.begin(115200);
  Serial.println("Boot");

  /*
  WiFi.mode(WIFI_AP); // Access Point Mode
  WiFi.softAP(ssid, password); 
  */

  //WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.print("Connecting to ");
  Serial.println(ssid);

  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(1000);
  }
  Serial.println();

  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Display page
  server.on("/", handleRoot);
  server.on("/readADC", handleADC);

  server.begin();
  Serial.println("Server started");
}

void loop()
{
  server.handleClient();
  delay(1);
}