#include "sketch.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

static int g_next_sketch_id = 1;
static int g_next_entity_id = 1;
static int g_next_constraint_id = 1;

// ============================================================================
// Sketch Creation & Cleanup
// ============================================================================

Sketch* sketch_create(const char* name, SketchPlane plane)
{
    Sketch* sketch = (Sketch*)calloc(1, sizeof(Sketch));
    if (!sketch) return NULL;

    sketch->id = g_next_sketch_id++;
    sketch->plane = plane;
    
    if (name) {
        strncpy(sketch->name, name, sizeof(sketch->name) - 1);
        sketch->name[sizeof(sketch->name) - 1] = '\0';
    } else {
        snprintf(sketch->name, sizeof(sketch->name), "Sketch_%d", sketch->id);
    }

    sketch->entities = NULL;
    sketch->entity_count = 0;
    sketch->entity_capacity = 0;

    sketch->constraints = NULL;
    sketch->constraint_count = 0;
    sketch->constraint_capacity = 0;

    sketch->is_2d_constrained = false;
    sketch->is_fully_constrained = false;
    sketch->needs_solver = false;

    return sketch;
}

void sketch_free(Sketch* sketch)
{
    if (!sketch) return;

    for (int i = 0; i < sketch->entity_count; i++) {
        free(sketch->entities[i]);
    }
    free(sketch->entities);
    free(sketch->constraints);
    free(sketch);
}

// ============================================================================
// Entity Management
// ============================================================================

static SketchEntity* sketch_add_entity_internal(Sketch* sketch, SketchEntity* entity)
{
    if (!sketch || !entity) return NULL;

    if (sketch->entity_count >= sketch->entity_capacity) {
        int new_capacity = sketch->entity_capacity == 0 ? 8 : sketch->entity_capacity * 2;
        void** new_entities = (void**)realloc(sketch->entities, new_capacity * sizeof(void*));
        if (!new_entities) {
            free(entity);
            return NULL;
        }
        sketch->entities = new_entities;
        sketch->entity_capacity = new_capacity;
    }

    entity->id = g_next_entity_id++;
    sketch->entities[sketch->entity_count++] = entity;
    sketch->needs_solver = true;

    return entity;
}

SketchPoint* sketch_add_point(Sketch* sketch, Vec2 pos)
{
    if (!sketch) return NULL;

    SketchPoint* pt = (SketchPoint*)calloc(1, sizeof(SketchPoint));
    if (!pt) return NULL;

    pt->base.type = SKETCH_POINT;
    pt->base.selected = false;
    pt->base.construction = false;
    pt->pos = pos;

    return (SketchPoint*)sketch_add_entity_internal(sketch, (SketchEntity*)pt);
}

SketchLine* sketch_add_line(Sketch* sketch, Vec2 p1, Vec2 p2)
{
    if (!sketch) return NULL;

    SketchLine* line = (SketchLine*)calloc(1, sizeof(SketchLine));
    if (!line) return NULL;

    line->base.type = SKETCH_LINE;
    line->base.selected = false;
    line->base.construction = false;
    line->p1 = p1;
    line->p2 = p2;

    return (SketchLine*)sketch_add_entity_internal(sketch, (SketchEntity*)line);
}

SketchCircle* sketch_add_circle(Sketch* sketch, Vec2 center, float radius)
{
    if (!sketch) return NULL;

    SketchCircle* circle = (SketchCircle*)calloc(1, sizeof(SketchCircle));
    if (!circle) return NULL;

    circle->base.type = SKETCH_CIRCLE;
    circle->base.selected = false;
    circle->base.construction = false;
    circle->center = center;
    circle->radius = radius;

    return (SketchCircle*)sketch_add_entity_internal(sketch, (SketchEntity*)circle);
}

SketchArc* sketch_add_arc(Sketch* sketch, Vec2 center, float radius, float start_angle, float end_angle)
{
    if (!sketch) return NULL;

    SketchArc* arc = (SketchArc*)calloc(1, sizeof(SketchArc));
    if (!arc) return NULL;

    arc->base.type = SKETCH_ARC;
    arc->base.selected = false;
    arc->base.construction = false;
    arc->center = center;
    arc->radius = radius;
    arc->start_angle = start_angle;
    arc->end_angle = end_angle;

    return (SketchArc*)sketch_add_entity_internal(sketch, (SketchEntity*)arc);
}

void sketch_remove_entity(Sketch* sketch, int entity_id)
{
    if (!sketch) return;

    for (int i = 0; i < sketch->entity_count; i++) {
        SketchEntity* entity = (SketchEntity*)sketch->entities[i];
        if (entity->id == entity_id) {
            free(entity);
            for (int j = i; j < sketch->entity_count - 1; j++) {
                sketch->entities[j] = sketch->entities[j + 1];
            }
            sketch->entity_count--;
            sketch->needs_solver = true;
            return;
        }
    }
}

SketchEntity* sketch_find_entity(Sketch* sketch, int entity_id)
{
    if (!sketch) return NULL;

    for (int i = 0; i < sketch->entity_count; i++) {
        SketchEntity* entity = (SketchEntity*)sketch->entities[i];
        if (entity->id == entity_id) {
            return entity;
        }
    }
    return NULL;
}

// ============================================================================
// Constraint Management
// ============================================================================

void sketch_add_constraint(Sketch* sketch, ConstraintType type, int entity_id_1, int entity_id_2, float value)
{
    if (!sketch) return;

    if (sketch->constraint_count >= sketch->constraint_capacity) {
        int new_capacity = sketch->constraint_capacity == 0 ? 8 : sketch->constraint_capacity * 2;
        SketchConstraint* new_constraints = (SketchConstraint*)realloc(sketch->constraints,
                                                                       new_capacity * sizeof(SketchConstraint));
        if (!new_constraints) return;
        sketch->constraints = new_constraints;
        sketch->constraint_capacity = new_capacity;
    }

    SketchConstraint* constraint = &sketch->constraints[sketch->constraint_count++];
    constraint->id = g_next_constraint_id++;
    constraint->type = type;
    constraint->entity_id_1 = entity_id_1;
    constraint->entity_id_2 = entity_id_2;
    constraint->value = value;

    sketch->needs_solver = true;
}

void sketch_remove_constraint(Sketch* sketch, int constraint_id)
{
    if (!sketch) return;

    for (int i = 0; i < sketch->constraint_count; i++) {
        if (sketch->constraints[i].id == constraint_id) {
            for (int j = i; j < sketch->constraint_count - 1; j++) {
                sketch->constraints[j] = sketch->constraints[j + 1];
            }
            sketch->constraint_count--;
            sketch->needs_solver = true;
            return;
        }
    }
}

// ============================================================================
// Sketch Solving
// ============================================================================

bool sketch_solve(Sketch* sketch)
{
    if (!sketch) return false;

    // TODO: Implement constraint solver (simple iterative approach for now)
    // For now, just mark as solved after basic validation
    sketch->needs_solver = false;
    
    // Simple check: if we have closed loops formed by lines, mark as constrained
    if (sketch->entity_count > 2) {
        sketch->is_2d_constrained = true;
    }

    return true;
}

bool sketch_is_fully_constrained(Sketch* sketch)
{
    if (!sketch) return false;
    return sketch->is_fully_constrained;
}

// ============================================================================
// Geometry Queries
// ============================================================================

ClosedLoop sketch_get_closed_loop(Sketch* sketch)
{
    ClosedLoop loop = {0};
    
    if (!sketch) return loop;

    // Simple implementation: collect all points from line segments
    // A more sophisticated version would trace connected lines to form closed loops

    int point_count = 0;
    for (int i = 0; i < sketch->entity_count; i++) {
        SketchEntity* entity = (SketchEntity*)sketch->entities[i];
        if (entity->type == SKETCH_POINT) point_count++;
        else if (entity->type == SKETCH_LINE) point_count += 2;
    }

    if (point_count < 3) return loop;  // Need at least 3 points for a closed loop

    loop.points = (Vec2*)malloc(point_count * sizeof(Vec2));
    if (!loop.points) return loop;

    loop.count = 0;

    // Collect points from entities
    for (int i = 0; i < sketch->entity_count; i++) {
        SketchEntity* entity = (SketchEntity*)sketch->entities[i];

        if (entity->type == SKETCH_POINT) {
            SketchPoint* pt = (SketchPoint*)entity;
            loop.points[loop.count++] = pt->pos;
        } else if (entity->type == SKETCH_LINE) {
            SketchLine* line = (SketchLine*)entity;
            loop.points[loop.count++] = line->p1;
            loop.points[loop.count++] = line->p2;
        }
    }

    return loop;
}

void sketch_free_loop(ClosedLoop loop)
{
    free(loop.points);
}
