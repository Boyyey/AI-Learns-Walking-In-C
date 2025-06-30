#include "nn.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Activation function
static float activation(float x) {
    return tanhf(x);
}

// Random float between -1 and 1
static float rand_float() {
    return ((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f;
}

NeuralNetwork* nn_create(int input, int hidden, int output) {
    NeuralNetwork* nn = (NeuralNetwork*)malloc(sizeof(NeuralNetwork));
    nn->input_count = input;
    nn->hidden_count = hidden;
    nn->output_count = output;

    nn->weights_ih = (float*)malloc(input * hidden * sizeof(float));
    nn->weights_ho = (float*)malloc(hidden * output * sizeof(float));
    nn->hidden_outputs = (float*)malloc(hidden * sizeof(float));
    
    // Initialize weights randomly
    for (int i = 0; i < input * hidden; i++) nn->weights_ih[i] = rand_float();
    for (int i = 0; i < hidden * output; i++) nn->weights_ho[i] = rand_float();

    return nn;
}

void nn_destroy(NeuralNetwork* nn) {
    free(nn->weights_ih);
    free(nn->weights_ho);
    free(nn->hidden_outputs);
    free(nn);
}

void nn_run(NeuralNetwork* nn, const float* inputs, float* outputs) {
    // Calculate hidden layer outputs
    for (int i = 0; i < nn->hidden_count; i++) {
        float sum = 0;
        for (int j = 0; j < nn->input_count; j++) {
            sum += inputs[j] * nn->weights_ih[j * nn->hidden_count + i];
        }
        nn->hidden_outputs[i] = activation(sum);
    }

    // Calculate final outputs
    for (int i = 0; i < nn->output_count; i++) {
        float sum = 0;
        for (int j = 0; j < nn->hidden_count; j++) {
            sum += nn->hidden_outputs[j] * nn->weights_ho[j * nn->output_count + i];
        }
        outputs[i] = activation(sum);
    }
}

void nn_set_weights(NeuralNetwork* nn, float* weights) {
    int w_ih_count = nn->input_count * nn->hidden_count;
    int w_ho_count = nn->hidden_count * nn->output_count;
    
    memcpy(nn->weights_ih, weights, w_ih_count * sizeof(float));
    memcpy(nn->weights_ho, &weights[w_ih_count], w_ho_count * sizeof(float));
}

void nn_get_weights_flat(NeuralNetwork* nn, float* weights_buffer) {
    int w_ih_count = nn->input_count * nn->hidden_count;
    int w_ho_count = nn->hidden_count * nn->output_count;

    memcpy(weights_buffer, nn->weights_ih, w_ih_count * sizeof(float));
    memcpy(&weights_buffer[w_ih_count], nn->weights_ho, w_ho_count * sizeof(float));
}
