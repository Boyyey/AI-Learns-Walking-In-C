#ifndef NN_H
#define NN_H

#include <stdlib.h>

typedef struct {
    int input_count;
    int hidden_count;
    int output_count;

    float* weights_ih; // Input to Hidden
    float* weights_ho; // Hidden to Output

    float* hidden_outputs;
} NeuralNetwork;

NeuralNetwork* nn_create(int input, int hidden, int output);
void nn_destroy(NeuralNetwork* nn);
void nn_run(NeuralNetwork* nn, const float* inputs, float* outputs);
void nn_set_weights(NeuralNetwork* nn, float* weights);
void nn_get_weights_flat(NeuralNetwork* nn, float* weights_buffer);

#endif // NN_H
