#include "raymarch_renderer.h"
#include "raylib.h"
#include "rlgl.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

// ============================================================================
// Renderer Lifecycle
// ============================================================================

RaymarchRenderer* raymarch_renderer_create(void)
{
    RaymarchRenderer* renderer = (RaymarchRenderer*)calloc(1, sizeof(RaymarchRenderer));
    if (!renderer) return NULL;

    // Load the raymarching shader
    char shader_path[512];
    snprintf(shader_path, sizeof(shader_path), "%s/raymarch.fs", GetWorkingDirectory());
    
    Shader sh = LoadShader(0, shader_path);
    renderer->shader = (void*)(intptr_t)sh.id;
    
    if (sh.id <= 0) {
        TraceLog(LOG_WARNING, "Failed to load raymarching shader from %s", shader_path);
    } else {
        TraceLog(LOG_INFO, "Loaded raymarching shader");
    }

    // Initialize default values
    renderer->camera_pos = (Vec3){5.0f, 3.0f, 5.0f};
    renderer->camera_target = (Vec3){0.0f, 0.0f, 0.0f};
    renderer->screen_width = 1280;
    renderer->screen_height = 720;
    
    renderer->max_raymarch_steps = 200;
    renderer->max_distance = 50.0f;
    renderer->surface_epsilon = 0.001f;
    
    renderer->sdf_root = NULL;

    return renderer;
}

void raymarch_renderer_free(RaymarchRenderer* renderer)
{
    if (!renderer) return;

    if (renderer->shader && (intptr_t)renderer->shader > 0) {
        Shader sh;
        sh.id = (unsigned int)(intptr_t)renderer->shader;
        UnloadShader(sh);
    }

    if (renderer->sdf_root) {
        sdf_free_tree(renderer->sdf_root);
    }

    free(renderer);
}

// ============================================================================
// Configuration
// ============================================================================

void raymarch_renderer_set_sdf_root(RaymarchRenderer* renderer, SDFNode* root)
{
    if (!renderer) return;

    if (renderer->sdf_root) {
        sdf_free_tree(renderer->sdf_root);
    }

    renderer->sdf_root = root;
}

void raymarch_renderer_set_camera(RaymarchRenderer* renderer, Vec3 pos, Vec3 target)
{
    if (!renderer) return;

    renderer->camera_pos = pos;
    renderer->camera_target = target;
}

void raymarch_renderer_set_resolution(RaymarchRenderer* renderer, int width, int height)
{
    if (!renderer) return;

    renderer->screen_width = width;
    renderer->screen_height = height;
}

// ============================================================================
// Rendering
// ============================================================================

void raymarch_renderer_draw(RaymarchRenderer* renderer)
{
    if (!renderer || !renderer->sdf_root) return;

    int shader_id = (intptr_t)renderer->shader;
    if (shader_id <= 0) {
        // Fallback: simple sphere
        DrawSphere((Vector3){0, 0, 0}, 1.0f, RED);
        return;
    }

    Shader sh;
    sh.id = (unsigned int)shader_id;

    BeginShaderMode(sh);

    // Set uniforms
    Vector2 res = {(float)renderer->screen_width, (float)renderer->screen_height};
    int res_loc = GetShaderLocation(sh, "resolution");
    SetShaderValue(sh, res_loc, &res, SHADER_UNIFORM_VEC2);

    float time = (float)GetTime();
    int time_loc = GetShaderLocation(sh, "utime");
    SetShaderValue(sh, time_loc, &time, SHADER_UNIFORM_FLOAT);

    Vector3 cam_pos = {renderer->camera_pos.x, renderer->camera_pos.y, renderer->camera_pos.z};
    int cam_pos_loc = GetShaderLocation(sh, "camPos");
    SetShaderValue(sh, cam_pos_loc, &cam_pos, SHADER_UNIFORM_VEC3);

    Vector3 cam_target = {renderer->camera_target.x, renderer->camera_target.y, renderer->camera_target.z};
    int cam_target_loc = GetShaderLocation(sh, "camTarget");
    SetShaderValue(sh, cam_target_loc, &cam_target, SHADER_UNIFORM_VEC3);

    // Draw fullscreen quad
    rlBegin(RL_QUADS);
    rlVertex2f(-1, -1);
    rlVertex2f(1, -1);
    rlVertex2f(1, 1);
    rlVertex2f(-1, 1);
    rlEnd();

    EndShaderMode();
}
