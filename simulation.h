#ifndef SIMULATION_H
#define SIMULATION_H

#include "physics.h"
#include "genetics.h"
#include <SDL.h>

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

#define NUM_POINTS 11
#define NUM_CONSTRAINTS 10

typedef struct {
    PointMass* points[NUM_POINTS];
    Constraint* constraints[NUM_CONSTRAINTS];
    int creature_index;

    // For fitness calculation
    float air_time;
    float total_upright_bonus;
    Vec2D start_pos;
} Biped;

typedef struct {
    Population* population;
    Biped* bipeds;
    int generation;
    float sim_time;
} SimulationState;

SimulationState* simulation_create();
void simulation_destroy(SimulationState* state);
void simulation_update(SimulationState* state, float dt);
void simulation_render(SimulationState* state, SDL_Renderer* renderer);

#endif // SIMULATION_H
