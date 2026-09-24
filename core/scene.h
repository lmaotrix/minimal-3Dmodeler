#ifndef SCENE_H
#define SCENE_H

#include "types.h"

// Node types in the scene graph
typedef enum {
    NODE_ROOT,
    NODE_SOLID,
    NODE_SKETCH,
    NODE_FEATURE,
    NODE_ASSEMBLY,
    NODE_REFERENCE_GEOMETRY
} NodeType;

// Feature types
typedef enum {
    FEATURE_EXTRUDE,
    FEATURE_REVOLVE,
    FEATURE_FILLET,
    FEATURE_CHAMFER,
    FEATURE_HOLE,
    FEATURE_PATTERN_LINEAR,
    FEATURE_PATTERN_CIRCULAR,
    FEATURE_BOOLEAN_UNION,
    FEATURE_BOOLEAN_SUBTRACT,
    FEATURE_BOOLEAN_INTERSECT
} FeatureType;

// Scene node - base class for all scene elements
typedef struct SceneNode {
    int id;
    char name[64];
    NodeType type;
    bool visible;
    bool selectable;
    Mat4 transform;  // Local transform
    
    // Bounding box (in local space)
    Vec3 bbox_min;
    Vec3 bbox_max;
    
    // Parent and children
    struct SceneNode* parent;
    struct SceneNode** children;
    int child_count;
    int child_capacity;
    
    // User data (type-specific)
    void* data;
    
    // Methods (function pointers)
    void (*update)(struct SceneNode* node);
    void (*draw)(struct SceneNode* node);
    void (*free_data)(void* data);
} SceneNode;

// Scene document
typedef struct {
    char name[128];
    char path[256];
    UnitSystem units;
    
    // Scene graph root
    SceneNode* root;
    
    // Current selection
    SceneNode** selected_nodes;
    int selected_count;
    int selected_capacity;
    
    // Camera
    struct {
        Vec3 position;
        Vec3 target;
        float fov;
        float near_clip;
        float far_clip;
    } camera;
    
    // View settings
    DisplayMode display_mode;
    bool show_grid;
    bool show_axes;
    bool show_origin;
    
    // History (undo/redo)
    struct {
        void** states;
        int current_index;
        int capacity;
    } history;
    
    // Flags
    bool needs_rebuild;
    bool is_modified;
} SceneDocument;

// ============================================================================
// Scene Node Functions
// ============================================================================

SceneNode* scene_create_node(NodeType type, const char* name);
void scene_add_child(SceneNode* parent, SceneNode* child);
void scene_remove_child(SceneNode* parent, SceneNode* child);

// ============================================================================
// Scene Document Functions
// ============================================================================

SceneDocument* scene_create_document(const char* name);
void scene_free_document(SceneDocument* doc);
void scene_update(SceneDocument* doc);
void scene_draw(SceneDocument* doc);
void scene_rebuild(SceneDocument* doc);

// ============================================================================
// Selection Functions
// ============================================================================

void scene_select_node(SceneDocument* doc, SceneNode* node);
void scene_deselect_node(SceneDocument* doc, SceneNode* node);
void scene_clear_selection(SceneDocument* doc);
bool scene_is_selected(SceneDocument* doc, SceneNode* node);

// ============================================================================
// Scene Graph Traversal
// ============================================================================

SceneNode* scene_find_node_by_name(SceneDocument* doc, const char* name);
SceneNode* scene_find_node_by_id(SceneDocument* doc, int id);

// ============================================================================
// Serialization
// ============================================================================

bool scene_save_document(SceneDocument* doc, const char* path);
SceneDocument* scene_load_document(const char* path);

#endif
