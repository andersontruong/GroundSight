#include "tflm_class.h"
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/version.h"

TFLM_Net::TFLM_Net(const void *modelData, int kArenaSize)
{
    error_reporter = new tflite::MicroErrorReporter();

    model = tflite::GetModel(modelData);
    if (model->version() != TFLITE_SCHEMA_VERSION)
    {
        TF_LITE_REPORT_ERROR(error_reporter, "Model provided is schema version %d not equal to supported version %d.",
                             model->version(), TFLITE_SCHEMA_VERSION);
        return;
    }
    // This pulls in the operators implementations we need
    resolver = new tflite::AllOpsResolver();

    tensor_arena = (uint8_t *)malloc(kArenaSize);
    if (!tensor_arena)
    {
        TF_LITE_REPORT_ERROR(error_reporter, "Could not allocate arena");
        return;
    }

    // Build an interpreter to run the model with.
    interpreter = new tflite::MicroInterpreter(
        model, *resolver, tensor_arena, kArenaSize, error_reporter);

    // Allocate memory from the tensor_arena for the model's tensors.
    TfLiteStatus allocate_status = interpreter->AllocateTensors();
    if (allocate_status != kTfLiteOk)
    {
        TF_LITE_REPORT_ERROR(error_reporter, "AllocateTensors() failed");
        return;
    }

    size_t used_bytes = interpreter->arena_used_bytes();
    TF_LITE_REPORT_ERROR(error_reporter, "Used bytes %d\n", used_bytes);

    // Obtain pointers to the model's input and output tensors.
    input = interpreter->input(0);
    output = interpreter->output(0);

    in_float = input->data.f;
    out_float = output->data.f;
}

void TFLM_Net::load_input(uint8_t* buffer, int size)
{
    for (size_t i = 0; i < size; i++)
    {
        input->data.f[i] = buffer[i] / 255.;
    }
}

float* TFLM_Net::output_float()
{
    return output->data.f;
}

int8_t* TFLM_Net::output_int8()
{
    return output->data.int8;
}

void TFLM_Net::run()
{
    interpreter->Invoke();
}
