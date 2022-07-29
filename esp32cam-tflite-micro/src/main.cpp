#include <Arduino.h>
#include "tflm_class.h"
#include "digit.h"
#include "model_data.h"

TFLM_Net *nn;

void setup()
{
  Serial.begin(115200);
  nn = new TFLM_Net(converted_model_tflite, 1028*60);
}

void loop()
{
  nn->load_input(digit, 784);
  nn->run();

  for (size_t i = 0; i < 10; i++)
  {
    Serial.print(nn->output_int8()[i]);
    Serial.print(", ");
  }
  Serial.println('\n');

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
  delay(1000);
}