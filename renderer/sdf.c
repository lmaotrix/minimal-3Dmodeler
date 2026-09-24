#include "sdf.h"
#include <stdlib.h>
#include <math.h>

// ============================================================================
// SDF Primitive Functions
// ============================================================================

float sdf_sphere(Vec3 p, float radius)
{
    return vec3_length(p) - radius;
}

float sdf_box(Vec3 p, Vec3 size)
{
    Vec3 q = (Vec3){
        fabsf(p.x) - size.x * 0.5f,
        fabsf(p.y) - size.y * 0.5f,
        fabsf(p.z) - size.z * 0.5f
    };
    
    float outside = vec3_length((Vec3){
        q.x > 0 ? q.x : 0,
        q.y > 0 ? q.y : 0,
        q.z > 0 ? q.z : 0
    });
    float inside = fminf(fmaxf(q.x, fmaxf(q.y, q.z)), 0.0f);
    
    return outside + inside;
}

float sdf_cylinder(Vec3 p, float radius, float height)
{
    float dx = sqrtf(p.x * p.x + p.z * p.z) - radius;
    float dy = fabsf(p.y) - height * 0.5f;
    
    float outside = sqrtf(dx > 0 ? dx * dx : 0) + sqrtf(dy > 0 ? dy * dy : 0);
    float inside = fminf(fmaxf(dx, dy), 0.0f);
    
    return outside + inside;
}

float sdf_cone(Vec3 p, float radius, float height)
{
    float h = height * 0.5f;
    float q = sqrtf(p.x * p.x + p.z * p.z);
    
    float dist_to_axis = q * (h / (radius + h)) - (h * radius) / (radius + h);
    
    return fmaxf(dist_to_axis, fmaxf(p.y - h, -(p.y + h)));
}

float sdf_torus(Vec3 p, float radius, float thickness)
{
    float q = sqrtf(p.x * p.x + p.z * p.z) - radius;
    return sqrtf(q * q + p.y * p.y) - thickness;
}

float sdf_plane(Vec3 p, Vec3 normal, float offset)
{
    return vec3_dot(p, normal) + offset;
}

// ============================================================================
// SDF Operations (CSG)
// ============================================================================

static float sdf_op_union(float a, float b)
{
    return fminf(a, b);
}

static float sdf_op_subtract(float a, float b)
{
    return fmaxf(a, -b);
}

static float sdf_op_intersect(float a, float b)
{
    return fmaxf(a, b);
}

static float sdf_op_smooth_union(float a, float b, float blend)
{
    float h = fmaxf(blend - fabsf(a - b), 0.0f);
    return fminf(a, b) - h * h * 0.25f / blend;
}

static float sdf_op_smooth_subtract(float a, float b, float blend)
{
    float h = fmaxf(blend - fabsf(a + b), 0.0f);
    return fmaxf(a, -b) - h * h * 0.25f / blend;
}

static float sdf_op_smooth_intersect(float a, float b, float blend)
{
    float h = fmaxf(blend - fabsf(a - b), 0.0f);
    return fmaxf(a, b) + h * h * 0.25f / blend;
}

// ============================================================================
// Matrix Inversion
// ============================================================================

static Mat4 mat4_inverse(Mat4 m)
{
    // Simplified inverse for affine transforms
    Mat4 inv = mat4_identity();
    
    // For now, just do a basic inversion
    // A full implementation would use Gaussian elimination
    float det = m.m[0] * (m.m[5] * m.m[10] - m.m[9] * m.m[6])
              - m.m[4] * (m.m[1] * m.m[10] - m.m[9] * m.m[2])
              + m.m[8] * (m.m[1] * m.m[6] - m.m[5] * m.m[2]);
    
    if (fabsf(det) < 1e-6f) return mat4_identity();
    
    // Simplified: just negate translation for now
    inv = m;
    inv.m[12] = -m.m[12];
    inv.m[13] = -m.m[13];
    inv.m[14] = -m.m[14];
    
    return inv;
}

// ============================================================================
// SDF Node Creation
// ============================================================================

SDFNode* sdf_create_sphere(float radius)
{
    SDFNode* node = (SDFNode*)calloc(1, sizeof(SDFNode));
    if (!node) return NULL;
    
    node->op_type = SDF_PRIMITIVE;
    node->data.primitive.type = SDF_SPHERE;
    node->data.primitive.params.sphere.radius = radius;
    node->data.primitive.transform = mat4_identity();
    node->data.primitive.inv_transform = mat4_identity();
    
    return node;
}

SDFNode* sdf_create_box(Vec3 size)
{
    SDFNode* node = (SDFNode*)calloc(1, sizeof(SDFNode));
    if (!node) return NULL;
    
    node->op_type = SDF_PRIMITIVE;
    node->data.primitive.type = SDF_BOX;
    node->data.primitive.params.box.size = size;
    node->data.primitive.transform = mat4_identity();
    node->data.primitive.inv_transform = mat4_identity();
    
    return node;
}

SDFNode* sdf_create_cylinder(float radius, float height)
{
    SDFNode* node = (SDFNode*)calloc(1, sizeof(SDFNode));
    if (!node) return NULL;
    
    node->op_type = SDF_PRIMITIVE;
    node->data.primitive.type = SDF_CYLINDER;
    node->data.primitive.params.cylinder.radius = radius;
    node->data.primitive.params.cylinder.height = height;
    node->data.primitive.transform = mat4_identity();
    node->data.primitive.inv_transform = mat4_identity();
    
    return node;
}

SDFNode* sdf_create_cone(float radius, float height)
{
    SDFNode* node = (SDFNode*)calloc(1, sizeof(SDFNode));
    if (!node) return NULL;
    
    node->op_type = SDF_PRIMITIVE;
    node->data.primitive.type = SDF_CONE;
    node->data.primitive.params.cone.radius = radius;
    node->data.primitive.params.cone.height = height;
    node->data.primitive.transform = mat4_identity();
    node->data.primitive.inv_transform = mat4_identity();
    
    return node;
}

SDFNode* sdf_create_torus(float radius, float thickness)
{
    SDFNode* node = (SDFNode*)calloc(1, sizeof(SDFNode));
    if (!node) return NULL;
    
    node->op_type = SDF_PRIMITIVE;
    node->data.primitive.type = SDF_TORUS;
    node->data.primitive.params.torus.radius = radius;
    node->data.primitive.params.torus.thickness = thickness;
    node->data.primitive.transform = mat4_identity();
    node->data.primitive.inv_transform = mat4_identity();
    
    return node;
}

SDFNode* sdf_create_plane(Vec3 normal, float offset)
{
    SDFNode* node = (SDFNode*)calloc(1, sizeof(SDFNode));
    if (!node) return NULL;
    
    node->op_type = SDF_PRIMITIVE;
    node->data.primitive.type = SDF_PLANE;
    node->data.primitive.params.plane.normal = vec3_normalize(normal);
    node->data.primitive.params.plane.offset = offset;
    node->data.primitive.transform = mat4_identity();
    node->data.primitive.inv_transform = mat4_identity();
    
    return node;
}

// ============================================================================
// SDF Operations
// ============================================================================

SDFNode* sdf_union(SDFNode* a, SDFNode* b)
{
    if (!a || !b) return a ? a : b;
    
    SDFNode* node = (SDFNode*)calloc(1, sizeof(SDFNode));
    if (!node) return NULL;
    
    node->op_type = SDF_UNION;
    node->data.op.left = a;
    node->data.op.right = b;
    node->data.op.blend_factor = 0.0f;
    
    return node;
}

SDFNode* sdf_subtract(SDFNode* a, SDFNode* b)
{
    if (!a || !b) return a;
    
    SDFNode* node = (SDFNode*)calloc(1, sizeof(SDFNode));
    if (!node) return NULL;
    
    node->op_type = SDF_SUBTRACTION;
    node->data.op.left = a;
    node->data.op.right = b;
    node->data.op.blend_factor = 0.0f;
    
    return node;
}

SDFNode* sdf_intersect(SDFNode* a, SDFNode* b)
{
    if (!a || !b) return a ? a : b;
    
    SDFNode* node = (SDFNode*)calloc(1, sizeof(SDFNode));
    if (!node) return NULL;
    
    node->op_type = SDF_INTERSECTION;
    node->data.op.left = a;
    node->data.op.right = b;
    node->data.op.blend_factor = 0.0f;
    
    return node;
}

SDFNode* sdf_smooth_union(SDFNode* a, SDFNode* b, float blend)
{
    if (!a || !b) return a ? a : b;
    
    SDFNode* node = (SDFNode*)calloc(1, sizeof(SDFNode));
    if (!node) return NULL;
    
    node->op_type = SDF_SMOOTH_UNION;
    node->data.op.left = a;
    node->data.op.right = b;
    node->data.op.blend_factor = blend;
    
    return node;
}

SDFNode* sdf_smooth_subtract(SDFNode* a, SDFNode* b, float blend)
{
    if (!a || !b) return a;
    
    SDFNode* node = (SDFNode*)calloc(1, sizeof(SDFNode));
    if (!node) return NULL;
    
    node->op_type = SDF_SMOOTH_SUBTRACTION;
    node->data.op.left = a;
    node->data.op.right = b;
    node->data.op.blend_factor = blend;
    
    return node;
}

SDFNode* sdf_smooth_intersect(SDFNode* a, SDFNode* b, float blend)
{
    if (!a || !b) return a ? a : b;
    
    SDFNode* node = (SDFNode*)calloc(1, sizeof(SDFNode));
    if (!node) return NULL;
    
    node->op_type = SDF_SMOOTH_INTERSECTION;
    node->data.op.left = a;
    node->data.op.right = b;
    node->data.op.blend_factor = blend;
    
    return node;
}

// ============================================================================
// Transform
// ============================================================================

void sdf_set_transform(SDFNode* node, Mat4 transform)
{
    if (!node || node->op_type != SDF_PRIMITIVE) return;
    
    node->data.primitive.transform = transform;
    node->data.primitive.inv_transform = mat4_inverse(transform);
}

// ============================================================================
// SDF Evaluation
// ============================================================================

static float sdf_evaluate_recursive(SDFNode* node, Vec3 p)
{
    if (!node) return 1e10f;
    
    if (node->op_type == SDF_PRIMITIVE) {
        // Transform point to local space
        Vec3 p_local = mat4_transform_point(node->data.primitive.inv_transform, p);
        
        switch (node->data.primitive.type) {
            case SDF_SPHERE:
                return sdf_sphere(p_local, node->data.primitive.params.sphere.radius);
            case SDF_BOX:
                return sdf_box(p_local, node->data.primitive.params.box.size);
            case SDF_CYLINDER:
                return sdf_cylinder(p_local, node->data.primitive.params.cylinder.radius, node->data.primitive.params.cylinder.height);
            case SDF_CONE:
                return sdf_cone(p_local, node->data.primitive.params.cone.radius, node->data.primitive.params.cone.height);
            case SDF_TORUS:
                return sdf_torus(p_local, node->data.primitive.params.torus.radius, node->data.primitive.params.torus.thickness);
            case SDF_PLANE:
                return sdf_plane(p_local, node->data.primitive.params.plane.normal, node->data.primitive.params.plane.offset);
            default:
                return 1e10f;
        }
    } else {
        float d_left = sdf_evaluate_recursive(node->data.op.left, p);
        float d_right = sdf_evaluate_recursive(node->data.op.right, p);
        
        switch (node->op_type) {
            case SDF_UNION:
                return sdf_op_union(d_left, d_right);
            case SDF_SUBTRACTION:
                return sdf_op_subtract(d_left, d_right);
            case SDF_INTERSECTION:
                return sdf_op_intersect(d_left, d_right);
            case SDF_SMOOTH_UNION:
                return sdf_op_smooth_union(d_left, d_right, node->data.op.blend_factor);
            case SDF_SMOOTH_SUBTRACTION:
                return sdf_op_smooth_subtract(d_left, d_right, node->data.op.blend_factor);
            case SDF_SMOOTH_INTERSECTION:
                return sdf_op_smooth_intersect(d_left, d_right, node->data.op.blend_factor);
            default:
                return sdf_op_union(d_left, d_right);
        }
    }
}

float sdf_evaluate(SDFNode* node, Vec3 p)
{
    return sdf_evaluate_recursive(node, p);
}

// ============================================================================
// Normal Estimation
// ============================================================================

Vec3 sdf_normal(SDFNode* node, Vec3 p, float eps)
{
    float d = sdf_evaluate(node, p);
    
    float dx = sdf_evaluate(node, vec3_add(p, (Vec3){eps, 0, 0})) - d;
    float dy = sdf_evaluate(node, vec3_add(p, (Vec3){0, eps, 0})) - d;
    float dz = sdf_evaluate(node, vec3_add(p, (Vec3){0, 0, eps})) - d;
    
    return vec3_normalize((Vec3){dx, dy, dz});
}

// ============================================================================
// Cleanup
// ============================================================================

void sdf_free_tree(SDFNode* node)
{
    if (!node) return;
    
    if (node->op_type != SDF_PRIMITIVE) {
        sdf_free_tree(node->data.op.left);
        sdf_free_tree(node->data.op.right);
    }
    
    free(node);
}
