#ifndef GENETICS_H
#define GENETICS_H

#include "nn.h"

typedef struct {
    NeuralNetwork* nn;
    float fitness;
} Creature;

typedef struct {
    Creature* creatures;
    int population_size;
    int gene_count;
    float mutation_rate;
} Population;

Population* ga_create_population(int size, int nn_inputs, int nn_hidden, int nn_outputs, float mutation_rate);
void ga_destroy_population(Population* pop);
void ga_evolve(Population* pop);

#endif // GENETICS_H
