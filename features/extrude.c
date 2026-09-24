#include "extrude.h"
#include <stdlib.h>
#include <string.h>

// Forward declare SDF functions (will be implemented separately)
struct SDFNode;
extern struct SDFNode* sdf_create_box(Vec3 size);
extern void sdf_set_transform(struct SDFNode* node, Mat4 transform);

// ============================================================================
// Extrude Feature Creation
// ============================================================================

static void extrude_free_data(void* data)
{
    // ExtrudeFeature is embedded, just clear
    if (data) memset(data, 0, sizeof(ExtrudeFeature));
}

static void extrude_update_node(SceneNode* node)
{
    if (!node || node->type != NODE_FEATURE) return;
    
    ExtrudeFeature* feat = (ExtrudeFeature*)node->data;
    if (!feat || !feat->params.sketch) return;

    // Update bounding box based on sketch + depth
    // TODO: Calculate accurate bbox from sketch geometry
    node->bbox_min = (Vec3){-2, -2, -feat->params.depth * 0.5f};
    node->bbox_max = (Vec3){2, 2, feat->params.depth * 0.5f};
}

static void extrude_draw_node(SceneNode* node)
{
    (void)node;
    // Drawing is handled by the raymarching renderer
    // This is called if we want to add debug visualizations
}

SceneNode* extrude_create_feature(Sketch* sketch, float depth, const char* name)
{
    if (!sketch || depth <= 0) return NULL;

    SceneNode* node = scene_create_node(NODE_FEATURE, name);
    if (!node) return NULL;

    ExtrudeFeature* feat = (ExtrudeFeature*)calloc(1, sizeof(ExtrudeFeature));
    if (!feat) {
        free(node);
        return NULL;
    }

    feat->params.sketch = sketch;
    feat->params.depth = depth;
    feat->params.symmetric = false;
    feat->params.taper_angle = 0.0f;
    feat->params.direction_mode = 0;
    feat->sketch_node = NULL;

    node->data = feat;
    node->update = extrude_update_node;
    node->draw = extrude_draw_node;
    node->free_data = extrude_free_data;

    return node;
}

void extrude_set_depth(SceneNode* feature_node, float depth)
{
    if (!feature_node || feature_node->type != NODE_FEATURE) return;
    
    ExtrudeFeature* feat = (ExtrudeFeature*)feature_node->data;
    if (feat && feature_node->parent) {
        feat->params.depth = depth;
    }
}

void extrude_set_symmetric(SceneNode* feature_node, bool symmetric)
{
    if (!feature_node || feature_node->type != NODE_FEATURE) return;
    
    ExtrudeFeature* feat = (ExtrudeFeature*)feature_node->data;
    if (feat) {
        feat->params.symmetric = symmetric;
    }
}

void extrude_set_taper(SceneNode* feature_node, float angle)
{
    if (!feature_node || feature_node->type != NODE_FEATURE) return;
    
    ExtrudeFeature* feat = (ExtrudeFeature*)feature_node->data;
    if (feat) {
        feat->params.taper_angle = angle;
    }
}

float extrude_get_depth(SceneNode* feature_node)
{
    if (!feature_node || feature_node->type != NODE_FEATURE) return 0.0f;
    
    ExtrudeFeature* feat = (ExtrudeFeature*)feature_node->data;
    return feat ? feat->params.depth : 0.0f;
}

bool extrude_get_symmetric(SceneNode* feature_node)
{
    if (!feature_node || feature_node->type != NODE_FEATURE) return false;
    
    ExtrudeFeature* feat = (ExtrudeFeature*)feature_node->data;
    return feat ? feat->params.symmetric : false;
}

// ============================================================================
// Extrude to SDF Conversion
// ============================================================================

struct SDFNode* extrude_to_sdf(SceneNode* feature_node)
{
    if (!feature_node || feature_node->type != NODE_FEATURE) return NULL;

    ExtrudeFeature* feat = (ExtrudeFeature*)feature_node->data;
    if (!feat || !feat->params.sketch) return NULL;

    // For now, create a simple box that represents the extrusion
    // A full implementation would:
    // 1. Trace the sketch profile
    // 2. Create a polygon from the sketch loop
    // 3. Extrude that polygon to create the SDF

    float depth = feat->params.symmetric ? feat->params.depth : feat->params.depth * 2.0f;
    struct SDFNode* box = sdf_create_box((Vec3){2.0f, 2.0f, depth});

    // Apply transform from feature node
    sdf_set_transform(box, feature_node->transform);

    return box;
}
