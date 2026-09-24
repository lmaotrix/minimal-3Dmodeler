#ifndef MATH_UTILS_H
#define MATH_UTILS_H

#include "types.h"
#include <math.h>

// Constants
#define PI 3.14159265358979323846f
#define EPSILON 1e-6f
#define DEG_TO_RAD (PI / 180.0f)
#define RAD_TO_DEG (180.0f / PI)

// Vector operations
static inline Vec2 vec2_add(Vec2 a, Vec2 b) { return (Vec2){a.x + b.x, a.y + b.y}; }
static inline Vec2 vec2_sub(Vec2 a, Vec2 b) { return (Vec2){a.x - b.x, a.y - b.y}; }
static inline Vec2 vec2_mul(Vec2 a, float s) { return (Vec2){a.x * s, a.y * s}; }
static inline float vec2_dot(Vec2 a, Vec2 b) { return a.x * b.x + a.y * b.y; }
static inline float vec2_length(Vec2 a) { return sqrtf(a.x * a.x + a.y * a.y); }
static inline Vec2 vec2_normalize(Vec2 a) {
    float len = vec2_length(a);
    if (len < EPSILON) return (Vec2){0, 0};
    return (Vec2){a.x / len, a.y / len};
}

static inline Vec3 vec3_add(Vec3 a, Vec3 b) { return (Vec3){a.x + b.x, a.y + b.y, a.z + b.z}; }
static inline Vec3 vec3_sub(Vec3 a, Vec3 b) { return (Vec3){a.x - b.x, a.y - b.y, a.z - b.z}; }
static inline Vec3 vec3_mul(Vec3 a, float s) { return (Vec3){a.x * s, a.y * s, a.z * s}; }
static inline float vec3_dot(Vec3 a, Vec3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
static inline Vec3 vec3_cross(Vec3 a, Vec3 b) {
    return (Vec3){
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}
static inline float vec3_length(Vec3 a) { return sqrtf(a.x * a.x + a.y * a.y + a.z * a.z); }
static inline Vec3 vec3_normalize(Vec3 a) {
    float len = vec3_length(a);
    if (len < EPSILON) return (Vec3){0, 0, 0};
    return (Vec3){a.x / len, a.y / len, a.z / len};
}
static inline float vec3_distance(Vec3 a, Vec3 b) {
    return vec3_length(vec3_sub(b, a));
}

// Matrix operations
Mat4 mat4_identity(void);
Mat4 mat4_translate(float x, float y, float z);
Mat4 mat4_rotate_x(float angle);
Mat4 mat4_rotate_y(float angle);
Mat4 mat4_rotate_z(float angle);
Mat4 mat4_scale(float x, float y, float z);
Mat4 mat4_multiply(Mat4 a, Mat4 b);
Vec3 mat4_transform_point(Mat4 m, Vec3 p);
Vec3 mat4_transform_vector(Mat4 m, Vec3 v);

#endif
