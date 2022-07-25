#ifndef _IMU_BUFFER_H_
#define _IMU_BUFFER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef struct _imu_buffer_t {
  uint8_t size;
  uint8_t index;
  int16_t *queue;
} imu_buffer_t;

imu_buffer_t* imu_buffer_init(uint8_t buffer_size);

void imu_buffer_pushback(imu_buffer_t *buff, int16_t insert_value);

int16_t imu_buffer_read(imu_buffer_t *buff, uint8_t buffer_index);

// Dot product of IIR filter coefficients with buffer
float filter_dot(imu_buffer_t *buffer1, float *coeff);

int16_t imu_filter(int16_t input, imu_buffer_t* x_inputs, float* x_coeff, imu_buffer_t* y_outputs, float* y_coeff);

#ifdef __cplusplus
}
#endif

#endif /* _IMU_BUFFER_H_ */