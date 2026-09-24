#ifndef SDF_H
#define SDF_H

#include "../core/types.h"
#include "../core/math_utils.h"

// SDF operation types (CSG operations)
typedef enum {
    SDF_PRIMITIVE,
    SDF_UNION,
    SDF_SUBTRACTION,
    SDF_INTERSECTION,
    SDF_SMOOTH_UNION,
    SDF_SMOOTH_SUBTRACTION,
    SDF_SMOOTH_INTERSECTION
} SDFOperation;

// SDF primitive types
typedef enum {
    SDF_SPHERE,
    SDF_BOX,
    SDF_CYLINDER,
    SDF_CONE,
    SDF_TORUS,
    SDF_PLANE
} SDFPrimitiveType;

// Base SDF node
typedef struct SDFNode {
    SDFOperation op_type;
    
    union {
        struct {
            SDFPrimitiveType type;
            
            // Primitive parameters
            union {
                struct { float radius; } sphere;
                struct { Vec3 size; } box;
                struct { float radius, height; } cylinder;
                struct { float radius, height; } cone;
                struct { float radius, thickness; } torus;
                struct { Vec3 normal; float offset; } plane;
            } params;
            
            Mat4 transform;  // Local transform
            Mat4 inv_transform;  // Inverse transform for SDF evaluation
        } primitive;
        
        struct {
            struct SDFNode* left;
            struct SDFNode* right;
            float blend_factor;  // For smooth operations
        } op;
    } data;
} SDFNode;

// SDF evaluation functions
float sdf_evaluate(SDFNode* node, Vec3 p);
Vec3 sdf_normal(SDFNode* node, Vec3 p, float eps);

// Node creation functions
SDFNode* sdf_create_sphere(float radius);
SDFNode* sdf_create_box(Vec3 size);
SDFNode* sdf_create_cylinder(float radius, float height);
SDFNode* sdf_create_cone(float radius, float height);
SDFNode* sdf_create_torus(float radius, float thickness);
SDFNode* sdf_create_plane(Vec3 normal, float offset);

SDFNode* sdf_union(SDFNode* a, SDFNode* b);
SDFNode* sdf_subtract(SDFNode* a, SDFNode* b);
SDFNode* sdf_intersect(SDFNode* a, SDFNode* b);
SDFNode* sdf_smooth_union(SDFNode* a, SDFNode* b, float blend);
SDFNode* sdf_smooth_subtract(SDFNode* a, SDFNode* b, float blend);
SDFNode* sdf_smooth_intersect(SDFNode* a, SDFNode* b, float blend);

// Transform operations
void sdf_set_transform(SDFNode* node, Mat4 transform);

// Cleanup
void sdf_free_tree(SDFNode* node);

#endif
