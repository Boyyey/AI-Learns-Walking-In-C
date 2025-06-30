#include "genetics.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Helper to sort creatures by fitness (descending)
static int compare_creatures(const void* a, const void* b) {
    float fitness_a = ((Creature*)a)->fitness;
    float fitness_b = ((Creature*)b)->fitness;
    if (fitness_a < fitness_b) return 1;
    if (fitness_a > fitness_b) return -1;
    return 0;
}

Population* ga_create_population(int size, int nn_inputs, int nn_hidden, int nn_outputs, float mutation_rate) {
    Population* pop = (Population*)malloc(sizeof(Population));
    pop->population_size = size;
    pop->gene_count = nn_inputs * nn_hidden + nn_hidden * nn_outputs;
    pop->mutation_rate = mutation_rate;
    pop->creatures = (Creature*)malloc(size * sizeof(Creature));

    for (int i = 0; i < size; i++) {
        pop->creatures[i].nn = nn_create(nn_inputs, nn_hidden, nn_outputs);
        pop->creatures[i].fitness = 0;
    }
    return pop;
}

void ga_destroy_population(Population* pop) {
    for (int i = 0; i < pop->population_size; i++) {
        nn_destroy(pop->creatures[i].nn);
    }
    free(pop->creatures);
    free(pop);
}

void ga_evolve(Population* pop) {
    // Sort by fitness
    qsort(pop->creatures, pop->population_size, sizeof(Creature), compare_creatures);

    // Elitism: Keep the top 20%
    int elite_count = pop->population_size * 0.2;
    if (elite_count == 0 && pop->population_size > 0) elite_count = 1;
    
    float* new_weights_pop = (float*)malloc(pop->population_size * pop->gene_count * sizeof(float));

    // Crossover
    for (int i = elite_count; i < pop->population_size; i++) {
        Creature* parent1 = &pop->creatures[rand() % elite_count];
        Creature* parent2 = &pop->creatures[rand() % elite_count];
        
        float* parent1_genes = (float*)malloc(pop->gene_count * sizeof(float));
        float* parent2_genes = (float*)malloc(pop->gene_count * sizeof(float));
        nn_get_weights_flat(parent1->nn, parent1_genes);
        nn_get_weights_flat(parent2->nn, parent2_genes);
        
        int crossover_point = rand() % pop->gene_count;
        float* child_genes = &new_weights_pop[i * pop->gene_count];

        memcpy(child_genes, parent1_genes, crossover_point * sizeof(float));
        memcpy(&child_genes[crossover_point], &parent2_genes[crossover_point], (pop->gene_count - crossover_point) * sizeof(float));

        free(parent1_genes);
        free(parent2_genes);
    }
    
    // Mutation
    for (int i = elite_count; i < pop->population_size; i++) {
        for (int j = 0; j < pop->gene_count; j++) {
            if ((float)rand() / RAND_MAX < pop->mutation_rate) {
                new_weights_pop[i * pop->gene_count + j] += (((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f) * 0.1f;
            }
        }
    }

    // Apply new weights and reset fitness
    for(int i = 0; i < pop->population_size; i++) {
        if (i >= elite_count) {
            nn_set_weights(pop->creatures[i].nn, &new_weights_pop[i * pop->gene_count]);
        }
        pop->creatures[i].fitness = 0;
    }
    
    free(new_weights_pop);
}
