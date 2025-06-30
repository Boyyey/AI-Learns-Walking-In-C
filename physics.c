#include "physics.h"
#include <math.h>
#include <stdlib.h>

#define GRAVITY 9.8f

PointMass* create_point_mass(Vec2D position, float mass) {
    PointMass* pm = (PointMass*)malloc(sizeof(PointMass));
    pm->position = position;
    pm->old_position = position;
    pm->acceleration = (Vec2D){0, 0};
    pm->mass = mass;
    return pm;
}

void update_point_mass(PointMass* pm, float dt) {
    if (pm->mass == 0.0f) return; // Infinite mass, static point

    Vec2D velocity = {
        (pm->position.x - pm->old_position.x),
        (pm->position.y - pm->old_position.y)
    };

    pm->old_position = pm->position;

    float dt2 = dt * dt;
    pm->position.x += velocity.x + pm->acceleration.x * dt2;
    pm->position.y += velocity.y + pm->acceleration.y * dt2;

    pm->acceleration = (Vec2D){0, 0};
}

void apply_force(PointMass* pm, Vec2D force) {
    if (pm->mass == 0.0f) return;
    pm->acceleration.x += force.x / pm->mass;
    pm->acceleration.y += force.y / pm->mass;
}

void satisfy_constraint(Constraint* c) {
    Vec2D delta = {
        c->p2->position.x - c->p1->position.x,
        c->p2->position.y - c->p1->position.y
    };

    float current_length = sqrtf(delta.x * delta.x + delta.y * delta.y);
    if (current_length == 0) current_length = 0.0001f; // Avoid division by zero

    float difference = (current_length - c->target_length) / current_length;
    
    Vec2D correction = {
        delta.x * 0.5f * difference,
        delta.y * 0.5f * difference
    };

    if (c->p1->mass != 0.0f) {
        c->p1->position.x += correction.x;
        c->p1->position.y += correction.y;
    }
    if (c->p2->mass != 0.0f) {
        c->p2->position.x -= correction.x;
        c->p2->position.y -= correction.y;
    }
}
