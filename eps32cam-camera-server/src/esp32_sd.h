#ifndef __ESP32_SD__
#define __ESP32_SD__

#ifdef __cplusplus
extern "C" {
#endif

int sd_init();

int sd_write(String path, const uint8_t *buf, size_t len);

#ifdef __cplusplus
}
#endif

#endif