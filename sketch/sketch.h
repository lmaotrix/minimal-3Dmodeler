#ifndef SKETCH_H
#define SKETCH_H

#include "../core/types.h"
#include <stdbool.h>

// Sketch entity types
typedef enum {
    SKETCH_POINT,
    SKETCH_LINE,
    SKETCH_CIRCLE,
    SKETCH_ARC,
    SKETCH_RECTANGLE,
    SKETCH_POLYGON
} SketchEntityType;

// Constraint types
typedef enum {
    CONSTRAINT_COINCIDENT,
    CONSTRAINT_COLLINEAR,
    CONSTRAINT_DISTANCE,
    CONSTRAINT_ANGLE,
    CONSTRAINT_PERPENDICULAR,
    CONSTRAINT_PARALLEL,
    CONSTRAINT_TANGENT,
    CONSTRAINT_HORIZONTAL,
    CONSTRAINT_VERTICAL,
    CONSTRAINT_EQUAL,
    CONSTRAINT_SYMMETRIC
} ConstraintType;

// Base sketch entity
typedef struct {
    int id;
    SketchEntityType type;
    bool selected;
    bool construction;
} SketchEntity;

// Point entity
typedef struct {
    SketchEntity base;
    Vec2 pos;
} SketchPoint;

// Line entity (from p1 to p2)
typedef struct {
    SketchEntity base;
    Vec2 p1;
    Vec2 p2;
} SketchLine;

// Circle entity
typedef struct {
    SketchEntity base;
    Vec2 center;
    float radius;
} SketchCircle;

// Arc entity
typedef struct {
    SketchEntity base;
    Vec2 center;
    float radius;
    float start_angle;
    float end_angle;
} SketchArc;

// Constraint
typedef struct {
    int id;
    ConstraintType type;
    int entity_id_1;
    int entity_id_2;
    float value;  // For distance/angle constraints
} SketchConstraint;

// Sketch plane (XY, XZ, YZ with optional offset)
typedef struct {
    Vec3 normal;
    Vec3 origin;
} SketchPlane;

// The sketch itself
typedef struct {
    int id;
    char name[64];
    SketchPlane plane;
    
    // Entities
    void** entities;  // Array of SketchEntity pointers
    int entity_count;
    int entity_capacity;
    
    // Constraints
    SketchConstraint* constraints;
    int constraint_count;
    int constraint_capacity;
    
    // Flags
    bool is_2d_constrained;
    bool is_fully_constrained;
    bool needs_solver;
} Sketch;

// ============================================================================
// Sketch Creation & Cleanup
// ============================================================================

Sketch* sketch_create(const char* name, SketchPlane plane);
void sketch_free(Sketch* sketch);

// ============================================================================
// Entity Management
// ============================================================================

SketchPoint* sketch_add_point(Sketch* sketch, Vec2 pos);
SketchLine* sketch_add_line(Sketch* sketch, Vec2 p1, Vec2 p2);
SketchCircle* sketch_add_circle(Sketch* sketch, Vec2 center, float radius);
SketchArc* sketch_add_arc(Sketch* sketch, Vec2 center, float radius, float start_angle, float end_angle);

void sketch_remove_entity(Sketch* sketch, int entity_id);
SketchEntity* sketch_find_entity(Sketch* sketch, int entity_id);

// ============================================================================
// Constraint Management
// ============================================================================

void sketch_add_constraint(Sketch* sketch, ConstraintType type, int entity_id_1, int entity_id_2, float value);
void sketch_remove_constraint(Sketch* sketch, int constraint_id);

// ============================================================================
// Sketch Solving & Solving State
// ============================================================================

bool sketch_solve(Sketch* sketch);
bool sketch_is_fully_constrained(Sketch* sketch);

// ============================================================================
// Geometry Queries
// ============================================================================

// Get all points that form a closed loop (for extrusion)
typedef struct {
    Vec2* points;
    int count;
} ClosedLoop;

ClosedLoop sketch_get_closed_loop(Sketch* sketch);
void sketch_free_loop(ClosedLoop loop);

#endif
