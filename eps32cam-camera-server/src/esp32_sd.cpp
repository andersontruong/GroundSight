#include "Arduino.h"
#include "esp32_sd.h"
#include "SD_MMC.h"

int sd_init()
{
  if(!SD_MMC.begin())
  {
    return 0;
  }

  if(SD_MMC.cardType() == CARD_NONE)
  {
    Serial.println("No SD Card attached");
    return 0;
  }
  return 1;
}

int sd_write(String path, const uint8_t *buf, size_t len)
{
  File file = SD_MMC.open(path.c_str(), FILE_WRITE);
  if (!file) return 0;
  file.write(buf, len);
  file.close();
  return 1;
}