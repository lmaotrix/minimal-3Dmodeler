#include "math_utils.h"
#include <math.h>

Mat4 mat4_identity(void) {
    Mat4 m = {{
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    }};
    return m;
}

Mat4 mat4_translate(float x, float y, float z) {
    Mat4 m = mat4_identity();
    m.m[12] = x;
    m.m[13] = y;
    m.m[14] = z;
    return m;
}

Mat4 mat4_rotate_x(float angle) {
    float c = cosf(angle);
    float s = sinf(angle);
    Mat4 m = mat4_identity();
    m.m[5] = c;
    m.m[6] = s;
    m.m[9] = -s;
    m.m[10] = c;
    return m;
}

Mat4 mat4_rotate_y(float angle) {
    float c = cosf(angle);
    float s = sinf(angle);
    Mat4 m = mat4_identity();
    m.m[0] = c;
    m.m[2] = -s;
    m.m[8] = s;
    m.m[10] = c;
    return m;
}

Mat4 mat4_rotate_z(float angle) {
    float c = cosf(angle);
    float s = sinf(angle);
    Mat4 m = mat4_identity();
    m.m[0] = c;
    m.m[1] = s;
    m.m[4] = -s;
    m.m[5] = c;
    return m;
}

Mat4 mat4_scale(float x, float y, float z) {
    Mat4 m = mat4_identity();
    m.m[0] = x;
    m.m[5] = y;
    m.m[10] = z;
    return m;
}

Mat4 mat4_multiply(Mat4 a, Mat4 b) {
    Mat4 result = {0};
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            float sum = 0;
            for (int k = 0; k < 4; k++) {
                sum += a.m[i * 4 + k] * b.m[k * 4 + j];
            }
            result.m[i * 4 + j] = sum;
        }
    }
    return result;
}

Vec3 mat4_transform_point(Mat4 m, Vec3 p) {
    Vec3 result;
    float w = m.m[3] * p.x + m.m[7] * p.y + m.m[11] * p.z + m.m[15];
    
    if (fabsf(w) > EPSILON) {
        w = 1.0f / w;
        result.x = (m.m[0] * p.x + m.m[4] * p.y + m.m[8] * p.z + m.m[12]) * w;
        result.y = (m.m[1] * p.x + m.m[5] * p.y + m.m[9] * p.z + m.m[13]) * w;
        result.z = (m.m[2] * p.x + m.m[6] * p.y + m.m[10] * p.z + m.m[14]) * w;
    } else {
        result = (Vec3){0, 0, 0};
    }
    
    return result;
}

Vec3 mat4_transform_vector(Mat4 m, Vec3 v) {
    Vec3 result;
    result.x = m.m[0] * v.x + m.m[4] * v.y + m.m[8] * v.z;
    result.y = m.m[1] * v.x + m.m[5] * v.y + m.m[9] * v.z;
    result.z = m.m[2] * v.x + m.m[6] * v.y + m.m[10] * v.z;
    return result;
}
