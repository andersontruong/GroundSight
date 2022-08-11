#ifndef __TFLM_NET__
#define __TFLM_NET__

#include <stdint.h>

namespace tflite
{
    class AllOpsResolver;
    class ErrorReporter;
    class Model;
    class MicroInterpreter;
}

struct TfLiteTensor;

class TFLM_Net
{
private:
    tflite::AllOpsResolver *resolver;
    tflite::ErrorReporter *error_reporter;
    const tflite::Model *model;
    tflite::MicroInterpreter *interpreter;
    TfLiteTensor *input;
    TfLiteTensor *output;
    uint8_t *tensor_arena;

public:
    TFLM_Net(const void *modelData, int kArenaSize);
    void load_input(uint8_t* buffer, int size);
    float* output_float();
    int8_t* output_int8();
    void run();

    float* out_float;
    float* in_float;
};

#endif