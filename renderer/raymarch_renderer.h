#ifndef RAYMARCH_RENDERER_H
#define RAYMARCH_RENDERER_H

#include "../core/types.h"
#include "sdf.h"

typedef struct {
    void* shader;  // Shader handle (raylib Shader)
    SDFNode* sdf_root;
    
    Vec3 camera_pos;
    Vec3 camera_target;
    
    int screen_width;
    int screen_height;
    
    // Debug/stats
    int max_raymarch_steps;
    float max_distance;
    float surface_epsilon;
} RaymarchRenderer;

// ============================================================================
// Renderer Lifecycle
// ============================================================================

RaymarchRenderer* raymarch_renderer_create(void);
void raymarch_renderer_free(RaymarchRenderer* renderer);

// ============================================================================
// Configuration
// ============================================================================

void raymarch_renderer_set_sdf_root(RaymarchRenderer* renderer, SDFNode* root);
void raymarch_renderer_set_camera(RaymarchRenderer* renderer, Vec3 pos, Vec3 target);
void raymarch_renderer_set_resolution(RaymarchRenderer* renderer, int width, int height);

// ============================================================================
// Rendering
// ============================================================================

void raymarch_renderer_draw(RaymarchRenderer* renderer);

#endif
