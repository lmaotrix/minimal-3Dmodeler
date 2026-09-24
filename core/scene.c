#include "scene.h"
#include "math_utils.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// ============================================================================
// Scene Node Implementation
// ============================================================================

static int g_next_node_id = 1;

SceneNode* scene_create_node(NodeType type, const char* name)
{
    SceneNode* node = (SceneNode*)calloc(1, sizeof(SceneNode));
    if (!node) return NULL;

    node->id = g_next_node_id++;
    node->type = type;
    node->visible = true;
    node->selectable = (type != NODE_ROOT);
    node->transform = mat4_identity();
    
    node->bbox_min = (Vec3){-1, -1, -1};
    node->bbox_max = (Vec3){1, 1, 1};
    
    node->parent = NULL;
    node->children = NULL;
    node->child_count = 0;
    node->child_capacity = 0;
    
    node->data = NULL;
    node->update = NULL;
    node->draw = NULL;
    node->free_data = NULL;
    
    if (name) {
        strncpy(node->name, name, sizeof(node->name) - 1);
        node->name[sizeof(node->name) - 1] = '\0';
    } else {
        snprintf(node->name, sizeof(node->name), "Node_%d", node->id);
    }
    
    return node;
}

void scene_add_child(SceneNode* parent, SceneNode* child)
{
    if (!parent || !child || child->parent != NULL) return;

    // Grow capacity if needed
    if (parent->child_count >= parent->child_capacity) {
        int new_capacity = parent->child_capacity == 0 ? 4 : parent->child_capacity * 2;
        SceneNode** new_children = (SceneNode**)realloc(parent->children, 
                                                        new_capacity * sizeof(SceneNode*));
        if (!new_children) return;
        
        parent->children = new_children;
        parent->child_capacity = new_capacity;
    }

    parent->children[parent->child_count++] = child;
    child->parent = parent;
}

void scene_remove_child(SceneNode* parent, SceneNode* child)
{
    if (!parent || !child) return;

    for (int i = 0; i < parent->child_count; i++) {
        if (parent->children[i] == child) {
            // Shift remaining children
            for (int j = i; j < parent->child_count - 1; j++) {
                parent->children[j] = parent->children[j + 1];
            }
            parent->child_count--;
            child->parent = NULL;
            return;
        }
    }
}

static void scene_free_node_recursive(SceneNode* node)
{
    if (!node) return;

    // Free children first
    for (int i = 0; i < node->child_count; i++) {
        scene_free_node_recursive(node->children[i]);
    }
    free(node->children);

    // Free node-specific data
    if (node->free_data && node->data) {
        node->free_data(node->data);
    }

    free(node);
}

// ============================================================================
// Scene Document Implementation
// ============================================================================

SceneDocument* scene_create_document(const char* name)
{
    SceneDocument* doc = (SceneDocument*)calloc(1, sizeof(SceneDocument));
    if (!doc) return NULL;

    if (name) {
        strncpy(doc->name, name, sizeof(doc->name) - 1);
        doc->name[sizeof(doc->name) - 1] = '\0';
    } else {
        strcpy(doc->name, "Untitled");
    }

    // Create root node
    doc->root = scene_create_node(NODE_ROOT, "Root");
    if (!doc->root) {
        free(doc);
        return NULL;
    }

    // Initialize camera
    doc->camera.position = (Vec3){5.0f, 3.0f, 5.0f};
    doc->camera.target = (Vec3){0.0f, 0.0f, 0.0f};
    doc->camera.fov = 60.0f;
    doc->camera.near_clip = 0.1f;
    doc->camera.far_clip = 1000.0f;

    // Initialize view settings
    doc->display_mode = DISPLAY_SHADED;
    doc->show_grid = true;
    doc->show_axes = true;
    doc->show_origin = true;

    // Initialize selection
    doc->selected_nodes = NULL;
    doc->selected_count = 0;
    doc->selected_capacity = 0;

    // Initialize history
    doc->history.states = NULL;
    doc->history.current_index = -1;
    doc->history.capacity = 0;

    doc->needs_rebuild = true;
    doc->is_modified = false;

    return doc;
}

void scene_free_document(SceneDocument* doc)
{
    if (!doc) return;

    scene_free_node_recursive(doc->root);
    free(doc->selected_nodes);
    free(doc->history.states);
    free(doc);
}

// ============================================================================
// Selection Management
// ============================================================================

void scene_select_node(SceneDocument* doc, SceneNode* node)
{
    if (!doc || !node || !node->selectable) return;

    // Check if already selected
    for (int i = 0; i < doc->selected_count; i++) {
        if (doc->selected_nodes[i] == node) return;
    }

    // Add to selection
    if (doc->selected_count >= doc->selected_capacity) {
        int new_capacity = doc->selected_capacity == 0 ? 4 : doc->selected_capacity * 2;
        SceneNode** new_selected = (SceneNode**)realloc(doc->selected_nodes,
                                                        new_capacity * sizeof(SceneNode*));
        if (!new_selected) return;

        doc->selected_nodes = new_selected;
        doc->selected_capacity = new_capacity;
    }

    doc->selected_nodes[doc->selected_count++] = node;
    doc->is_modified = true;
}

void scene_deselect_node(SceneDocument* doc, SceneNode* node)
{
    if (!doc || !node) return;

    for (int i = 0; i < doc->selected_count; i++) {
        if (doc->selected_nodes[i] == node) {
            for (int j = i; j < doc->selected_count - 1; j++) {
                doc->selected_nodes[j] = doc->selected_nodes[j + 1];
            }
            doc->selected_count--;
            doc->is_modified = true;
            return;
        }
    }
}

void scene_clear_selection(SceneDocument* doc)
{
    if (!doc) return;

    doc->selected_count = 0;
    doc->is_modified = true;
}

bool scene_is_selected(SceneDocument* doc, SceneNode* node)
{
    if (!doc || !node) return false;

    for (int i = 0; i < doc->selected_count; i++) {
        if (doc->selected_nodes[i] == node) return true;
    }
    return false;
}

// ============================================================================
// Scene Graph Operations
// ============================================================================

static void scene_update_node_recursive(SceneNode* node)
{
    if (!node) return;

    // Call node-specific update
    if (node->update) {
        node->update(node);
    }

    // Update children
    for (int i = 0; i < node->child_count; i++) {
        scene_update_node_recursive(node->children[i]);
    }
}

void scene_update(SceneDocument* doc)
{
    if (!doc) return;

    scene_update_node_recursive(doc->root);
}

static void scene_draw_node_recursive(SceneNode* node)
{
    if (!node || !node->visible) return;

    // Call node-specific draw
    if (node->draw) {
        node->draw(node);
    }

    // Draw children
    for (int i = 0; i < node->child_count; i++) {
        scene_draw_node_recursive(node->children[i]);
    }
}

void scene_draw(SceneDocument* doc)
{
    if (!doc) return;

    scene_draw_node_recursive(doc->root);
}

void scene_rebuild(SceneDocument* doc)
{
    if (!doc) return;

    doc->needs_rebuild = false;
    // This will be populated by the renderer to rebuild SDF tree, etc.
}

// ============================================================================
// Scene Graph Traversal Utilities
// ============================================================================

typedef struct {
    SceneNode** nodes;
    int count;
    int capacity;
} NodeArray;

static void collect_nodes_recursive(SceneNode* node, NodeArray* arr)
{
    if (!node) return;

    if (arr->count >= arr->capacity) {
        int new_capacity = arr->capacity == 0 ? 16 : arr->capacity * 2;
        SceneNode** new_nodes = (SceneNode**)realloc(arr->nodes, 
                                                     new_capacity * sizeof(SceneNode*));
        if (!new_nodes) return;
        
        arr->nodes = new_nodes;
        arr->capacity = new_capacity;
    }

    arr->nodes[arr->count++] = node;

    for (int i = 0; i < node->child_count; i++) {
        collect_nodes_recursive(node->children[i], arr);
    }
}

SceneNode* scene_find_node_by_name(SceneDocument* doc, const char* name)
{
    if (!doc || !name) return NULL;

    NodeArray arr = {0};
    collect_nodes_recursive(doc->root, &arr);

    for (int i = 0; i < arr.count; i++) {
        if (strcmp(arr.nodes[i]->name, name) == 0) {
            free(arr.nodes);
            return arr.nodes[i];
        }
    }

    free(arr.nodes);
    return NULL;
}

SceneNode* scene_find_node_by_id(SceneDocument* doc, int id)
{
    if (!doc) return NULL;

    NodeArray arr = {0};
    collect_nodes_recursive(doc->root, &arr);

    for (int i = 0; i < arr.count; i++) {
        if (arr.nodes[i]->id == id) {
            free(arr.nodes);
            return arr.nodes[i];
        }
    }

    free(arr.nodes);
    return NULL;
}

// ============================================================================
// Serialization Stubs (for later)
// ============================================================================

bool scene_save_document(SceneDocument* doc, const char* path)
{
    if (!doc || !path) return false;

    FILE* f = fopen(path, "wb");
    if (!f) return false;

    // TODO: Implement serialization
    fprintf(f, "CAD_DOCUMENT_V1\n");
    fprintf(f, "Name: %s\n", doc->name);

    fclose(f);
    doc->is_modified = false;
    return true;
}

SceneDocument* scene_load_document(const char* path)
{
    if (!path) return NULL;

    FILE* f = fopen(path, "rb");
    if (!f) return NULL;

    char header[32] = {0};
    fgets(header, sizeof(header), f);

    fclose(f);

    // TODO: Implement deserialization
    return scene_create_document("Loaded Document");
}
