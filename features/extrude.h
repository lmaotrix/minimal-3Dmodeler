#ifndef EXTRUDE_H
#define EXTRUDE_H

#include "../core/types.h"
#include "../core/scene.h"
#include "../sketch/sketch.h"

// Extrude feature parameters
typedef struct {
    Sketch* sketch;           // Source sketch
    float depth;              // Extrusion depth
    bool symmetric;           // Extrude symmetric about sketch plane
    float taper_angle;        // Taper angle (for drafts)
    int direction_mode;       // 0 = normal, 1 = up, 2 = down
} ExtrudeParams;

// Feature data attached to a feature node
typedef struct {
    ExtrudeParams params;
    SceneNode* sketch_node;   // Reference to sketch node in scene
} ExtrudeFeature;

// ============================================================================
// Extrude Feature Creation
// ============================================================================

// Create an extrude feature from a sketch
SceneNode* extrude_create_feature(Sketch* sketch, float depth, const char* name);

// Set extrude parameters
void extrude_set_depth(SceneNode* feature_node, float depth);
void extrude_set_symmetric(SceneNode* feature_node, bool symmetric);
void extrude_set_taper(SceneNode* feature_node, float angle);

// Get parameters
float extrude_get_depth(SceneNode* feature_node);
bool extrude_get_symmetric(SceneNode* feature_node);

// ============================================================================
// Extrude to SDF Conversion
// ============================================================================

// Convert extrude feature to SDF representation
// This will be called by the renderer to build the raymarching scene
struct SDFNode* extrude_to_sdf(SceneNode* feature_node);

#endif
