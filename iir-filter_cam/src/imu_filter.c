#include "imu_filter.h"
#include "stdlib.h"
#include "string.h"

imu_buffer_t* imu_buffer_init(uint8_t buffer_size) {
  imu_buffer_t *buff = (imu_buffer_t *)malloc(sizeof(imu_buffer_t));
  buff->size = buffer_size;
  buff->index = 0;
  buff->queue = (int16_t *)malloc(sizeof(*buff->queue) * buffer_size);
  memset(buff->queue, 0, sizeof(*buff->queue) * buffer_size);

  return buff;
}

void imu_buffer_pushback(imu_buffer_t *buff, int16_t insert_value) {
  if (++buff->index == buff->size) buff->index = 0;
  buff->queue[buff->index] = insert_value;
}

int16_t imu_buffer_read(imu_buffer_t *buff, uint8_t buffer_index) {
  int16_t access_index = buff->index;
  for (uint8_t i = 0; i < buffer_index; i++) {
    access_index--;
    if (access_index == -1)
      access_index = buff->size - 1;
  }
  return buff->queue[access_index];
}

// Dot product of IIR filter coefficients with buffer
float filter_dot(imu_buffer_t *buffer1, float *coeff) {
  float sum = 0;
  for (uint8_t i = 0; i < buffer1->size; i++, coeff++) {
    sum += imu_buffer_read(buffer1, i) * *coeff;
  }
  return sum;
}

int16_t imu_filter(int16_t input, imu_buffer_t* x_inputs, float* x_coeff, imu_buffer_t* y_outputs, float* y_coeff)
{
  int16_t output;
  imu_buffer_pushback(x_inputs, input);
  output = filter_dot(x_inputs, x_coeff) - filter_dot(y_outputs, y_coeff);
  imu_buffer_pushback(y_outputs, output);
  return output;
}

