#include "simulation.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define POPULATION_SIZE 50
#define NN_INPUTS 8
#define NN_HIDDEN 16
#define NN_OUTPUTS 4
#define GENE_COUNT (NN_INPUTS * NN_HIDDEN + NN_HIDDEN * NN_OUTPUTS)
#define MUTATION_RATE 0.05f

#define SIM_DURATION 10.0f // seconds
#define GROUND_Y (SCREEN_HEIGHT - 100)

static Biped create_biped(int creature_index, Vec2D start_pos);
static void destroy_biped(Biped* b);
static void reset_biped(Biped* b, Vec2D start_pos);

SimulationState* simulation_create() {
    SimulationState* state = (SimulationState*)malloc(sizeof(SimulationState));
    state->population = ga_create_population(POPULATION_SIZE, NN_INPUTS, NN_HIDDEN, NN_OUTPUTS, MUTATION_RATE);
    state->bipeds = (Biped*)malloc(POPULATION_SIZE * sizeof(Biped));
    state->generation = 1;
    state->sim_time = 0;

    for (int i = 0; i < POPULATION_SIZE; i++) {
        state->bipeds[i] = create_biped(i, (Vec2D){200, GROUND_Y - 100});
    }

    return state;
}

void simulation_destroy(SimulationState* state) {
    ga_destroy_population(state->population);
    for (int i = 0; i < POPULATION_SIZE; i++) {
        destroy_biped(&state->bipeds[i]);
    }
    free(state->bipeds);
    free(state);
}

void simulation_update(SimulationState* state, float dt) {
    state->sim_time += dt;

    for (int i = 0; i < POPULATION_SIZE; i++) {
        Biped* b = &state->bipeds[i];
        
        // --- NN Input Calculation ---
        float inputs[NN_INPUTS];
        PointMass* chest = b->points[1];
        PointMass* pelvis = b->points[2];
        PointMass* l_ankle = b->points[4];
        PointMass* r_ankle = b->points[6];
        PointMass* l_hand = b->points[8];
        PointMass* r_hand = b->points[10];

        // Input 0: Pelvis height from ground
        inputs[0] = (GROUND_Y - pelvis->position.y) / 100.f; 
        // Input 1: Torso angle (upright = 0)
        inputs[1] = atan2f(chest->position.x - pelvis->position.x, chest->position.y - pelvis->position.y);
        // Inputs 2,3: Foot positions relative to pelvis (y)
        inputs[2] = (l_ankle->position.y - pelvis->position.y) / 100.f;
        inputs[3] = (r_ankle->position.y - pelvis->position.y) / 100.f;
        // Inputs 4,5: Foot positions relative to pelvis (x)
        inputs[4] = (l_ankle->position.x - pelvis->position.x) / 100.f;
        inputs[5] = (r_ankle->position.x - pelvis->position.x) / 100.f;
        // Inputs 6,7: Hand positions relative to pelvis (y)
        inputs[6] = (l_hand->position.y - pelvis->position.y) / 100.f;
        inputs[7] = (r_hand->position.y - pelvis->position.y) / 100.f;

        // --- Run NN ---
        float outputs[NN_OUTPUTS];
        nn_run(state->population->creatures[i].nn, inputs, outputs);

        // --- Apply Forces from NN ---
        apply_force(l_ankle, (Vec2D){outputs[0] * 2000.f, 0});
        apply_force(r_ankle, (Vec2D){outputs[1] * 2000.f, 0});
        apply_force(l_hand, (Vec2D){outputs[2] * 1000.f, 0});
        apply_force(r_hand, (Vec2D){outputs[3] * 1000.f, 0});

        // --- Update Physics & Fitness States ---
        float upright_bonus = fmaxf(0, chest->position.y - pelvis->position.y); // Reward for chest over pelvis
        b->total_upright_bonus += upright_bonus * dt;

        if (l_ankle->position.y < GROUND_Y - 1 && r_ankle->position.y < GROUND_Y - 1) {
            b->air_time += dt;
        }

        for (int j = 0; j < NUM_POINTS; j++) {
            apply_force(b->points[j], (Vec2D){0, 50}); // Gravity
            update_point_mass(b->points[j], dt);
            // Ground collision
            if (b->points[j]->position.y > GROUND_Y) {
                b->points[j]->position.y = GROUND_Y;
            }
        }
        for (int j = 0; j < 5; j++) {
            for (int k = 0; k < NUM_CONSTRAINTS; k++) {
                satisfy_constraint(b->constraints[k]);
            }
        }
    }

    if (state->sim_time >= SIM_DURATION) {
        float total_fitness = 0, max_fitness = -1e9, min_fitness = 1e9;

        for (int i = 0; i < POPULATION_SIZE; i++) {
            Biped* b = &state->bipeds[i];
            float distance_moved = b->points[2]->position.x - b->start_pos.x;
            
            float fitness = distance_moved;
            fitness += b->total_upright_bonus * 5.0f; // Reward for staying upright
            fitness -= b->air_time * 20.0f; // Penalize jumping

            if (fitness < 0) fitness = 0;
            state->population->creatures[i].fitness = fitness;
            total_fitness += fitness;
            if (fitness > max_fitness) max_fitness = fitness;
            if (fitness < min_fitness) min_fitness = fitness;
        }
        
        ga_evolve(state->population);
        
        float avg_fitness = total_fitness / POPULATION_SIZE;
        printf("Generation %d | Avg Fitness: %.2f | Best: %.2f | Worst: %.2f\n",
               state->generation, avg_fitness, max_fitness, min_fitness);
        fflush(stdout);

        state->sim_time = 0;
        for (int i = 0; i < POPULATION_SIZE; i++) {
            reset_biped(&state->bipeds[i], (Vec2D){200, GROUND_Y - 100});
        }
        state->generation++;
    }
}

static void render_filled_circle(SDL_Renderer* renderer, int x, int y, int radius) {
    for (int dy = -radius; dy <= radius; dy++) {
        for (int dx = -radius; dx <= radius; dx++) {
            if (dx * dx + dy * dy <= radius * radius) {
                SDL_RenderDrawPoint(renderer, x + dx, y + dy);
            }
        }
    }
}

void simulation_render(SimulationState* state, SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    for (int i = 0; i < POPULATION_SIZE; i++) {
        Biped* b = &state->bipeds[i];
        // Render skeleton
        for (int j = 0; j < NUM_CONSTRAINTS; j++) {
            SDL_RenderDrawLine(renderer,
                b->constraints[j]->p1->position.x, b->constraints[j]->p1->position.y,
                b->constraints[j]->p2->position.x, b->constraints[j]->p2->position.y);
        }
        // Render head
        render_filled_circle(renderer, b->points[0]->position.x, b->points[0]->position.y, 8);
    }
    SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);
    SDL_RenderDrawLine(renderer, 0, GROUND_Y, SCREEN_WIDTH, GROUND_Y);
}

// Biped creation
static Biped create_biped(int creature_index, Vec2D start_pos) {
    Biped b;
    b.creature_index = creature_index;
    b.start_pos = start_pos;

    // Points
    b.points[0] = create_point_mass((Vec2D){start_pos.x, start_pos.y - 30}, 1.f);  // Head
    b.points[1] = create_point_mass((Vec2D){start_pos.x, start_pos.y}, 1.f);      // Chest
    b.points[2] = create_point_mass((Vec2D){start_pos.x, start_pos.y + 30}, 1.f);  // Pelvis
    b.points[3] = create_point_mass((Vec2D){start_pos.x - 15, start_pos.y + 60}, 1.f); // L Knee
    b.points[4] = create_point_mass((Vec2D){start_pos.x - 15, start_pos.y + 90}, 1.f); // L Ankle
    b.points[5] = create_point_mass((Vec2D){start_pos.x + 15, start_pos.y + 60}, 1.f); // R Knee
    b.points[6] = create_point_mass((Vec2D){start_pos.x + 15, start_pos.y + 90}, 1.f); // R Ankle
    b.points[7] = create_point_mass((Vec2D){start_pos.x - 20, start_pos.y + 30}, 1.f); // L Elbow
    b.points[8] = create_point_mass((Vec2D){start_pos.x - 20, start_pos.y + 60}, 1.f); // L Hand
    b.points[9] = create_point_mass((Vec2D){start_pos.x + 20, start_pos.y + 30}, 1.f); // R Elbow
    b.points[10]= create_point_mass((Vec2D){start_pos.x + 20, start_pos.y + 60}, 1.f); // R Hand
    
    // Constraints
    for (int i=0; i<NUM_CONSTRAINTS; ++i) b.constraints[i] = malloc(sizeof(Constraint));
    *b.constraints[0] = (Constraint){b.points[0], b.points[1], 30}; // Neck
    *b.constraints[1] = (Constraint){b.points[1], b.points[2], 30}; // Spine
    *b.constraints[2] = (Constraint){b.points[1], b.points[7], 30}; // L Upper Arm
    *b.constraints[3] = (Constraint){b.points[7], b.points[8], 30}; // L Forearm
    *b.constraints[4] = (Constraint){b.points[1], b.points[9], 30}; // R Upper Arm
    *b.constraints[5] = (Constraint){b.points[9], b.points[10], 30}; // R Forearm
    *b.constraints[6] = (Constraint){b.points[2], b.points[3], 30}; // L Thigh
    *b.constraints[7] = (Constraint){b.points[3], b.points[4], 30}; // L Shin
    *b.constraints[8] = (Constraint){b.points[2], b.points[5], 30}; // R Thigh
    *b.constraints[9] = (Constraint){b.points[5], b.points[6], 30}; // R Shin

    reset_biped(&b, start_pos);
    return b;
}

static void reset_biped(Biped* b, Vec2D start_pos) {
    b->start_pos = start_pos;
    b->air_time = 0;
    b->total_upright_bonus = 0;

    b->points[0]->position = (Vec2D){start_pos.x, start_pos.y - 30};
    b->points[1]->position = (Vec2D){start_pos.x, start_pos.y};
    b->points[2]->position = (Vec2D){start_pos.x, start_pos.y + 30};
    b->points[3]->position = (Vec2D){start_pos.x - 15, start_pos.y + 60};
    b->points[4]->position = (Vec2D){start_pos.x - 15, start_pos.y + 90};
    b->points[5]->position = (Vec2D){start_pos.x + 15, start_pos.y + 60};
    b->points[6]->position = (Vec2D){start_pos.x + 15, start_pos.y + 90};
    b->points[7]->position = (Vec2D){start_pos.x - 20, start_pos.y + 30};
    b->points[8]->position = (Vec2D){start_pos.x - 20, start_pos.y + 60};
    b->points[9]->position = (Vec2D){start_pos.x + 20, start_pos.y + 30};
    b->points[10]->position= (Vec2D){start_pos.x + 20, start_pos.y + 60};

    for(int i = 0; i < NUM_POINTS; i++) {
        b->points[i]->old_position = b->points[i]->position;
    }
}

static void destroy_biped(Biped* b) {
    for (int i = 0; i < NUM_POINTS; i++) {
        free(b->points[i]);
    }
    for (int i = 0; i < NUM_CONSTRAINTS; i++) {
        free(b->constraints[i]);
    }
}
