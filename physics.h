#ifndef PHYSICS_H
#define PHYSICS_H

typedef struct {
    float x;
    float y;
} Vec2D;

typedef struct {
    Vec2D position;
    Vec2D old_position;
    Vec2D acceleration;
    float mass;
} PointMass;

typedef struct {
    PointMass* p1;
    PointMass* p2;
    float target_length;
} Constraint;

PointMass* create_point_mass(Vec2D position, float mass);
void update_point_mass(PointMass* pm, float dt);
void apply_force(PointMass* pm, Vec2D force);
void satisfy_constraint(Constraint* c);

#endif // PHYSICS_H
