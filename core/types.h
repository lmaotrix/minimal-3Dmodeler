#ifndef TYPES_H
#define TYPES_H

#include <stdbool.h>
#include <stdint.h>

// Vector types
typedef struct {
    float x, y;
} Vec2;

typedef struct {
    float x, y, z;
} Vec3;

typedef struct {
    float x, y, z, w;
} Vec4;

typedef struct {
    float m[16];
} Mat4;

typedef struct {
    float m[9];
} Mat3;

// Common enumerations
typedef enum {
    VIEW_FRONT,
    VIEW_TOP,
    VIEW_RIGHT,
    VIEW_ISOMETRIC,
    VIEW_FREE
} ViewDirection;

typedef enum {
    DISPLAY_SHADED,
    DISPLAY_WIREFRAME,
    DISPLAY_SHADED_WITH_EDGES,
    DISPLAY_HIDDEN_LINES_REMOVED
} DisplayMode;

typedef enum {
    UNIT_MILLIMETER,
    UNIT_CENTIMETER,
    UNIT_METER,
    UNIT_INCH,
    UNIT_FOOT
} UnitSystem;

// Raymarched primitive types
typedef struct {
    Vec3 center;
    float radius;
} Sphere;

typedef struct {
    Vec3 min;
    Vec3 max;
} Box;

typedef struct {
    Vec3 start;
    Vec3 end;
    float radius;
} Cylinder;

#endif
