#include "raylib.h"
#include "rlgl.h"
#include "core/scene.h"
#include "core/math_utils.h"
#include "sketch/sketch.h"
#include "features/extrude.h"
#include "renderer/sdf.h"
#include "renderer/raymarch_renderer.h"
#include <stdio.h>
#include <math.h>

typedef struct {
    SceneDocument* doc;
    RaymarchRenderer* renderer;
    
    // Camera state
    float cam_yaw;
    float cam_pitch;
    float cam_distance;
    Vector3 cam_target;
    
    // UI state
    bool show_property_manager;
    bool show_feature_tree;
    
    // Application state
    int selected_tool;
    bool use_raymarching;
} Application;

static Application app = {0};

// Convert Vector3 to Vec3
static Vec3 v3_to_vec3(Vector3 v) {
    return (Vec3){v.x, v.y, v.z};
}

// ============================================================================
// Camera Update
// ============================================================================

static void update_app_camera(void)
{
    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
        Vector2 delta = GetMouseDelta();
        app.cam_yaw -= delta.x * 0.003f;
        app.cam_pitch -= delta.y * 0.003f;
        
        if (app.cam_pitch > 1.5f) app.cam_pitch = 1.5f;
        if (app.cam_pitch < -1.5f) app.cam_pitch = -1.5f;
    }
    
    app.cam_distance -= GetMouseWheelMove() * 0.5f;
    if (app.cam_distance < 1.0f) app.cam_distance = 1.0f;
    
    // Update renderer
    if (app.renderer && app.use_raymarching) {
        Vector3 cam_pos = {
            app.cam_target.x + cosf(app.cam_pitch) * cosf(app.cam_yaw) * app.cam_distance,
            app.cam_target.y + sinf(app.cam_pitch) * app.cam_distance,
            app.cam_target.z + cosf(app.cam_pitch) * sinf(app.cam_yaw) * app.cam_distance
        };
        raymarch_renderer_set_camera(app.renderer, v3_to_vec3(cam_pos), v3_to_vec3(app.cam_target));
    }
}

// ============================================================================
// UI Drawing
// ============================================================================

static void draw_ui(void)
{
    // Top toolbar
    DrawRectangle(0, 0, GetScreenWidth(), 40, ColorAlpha(DARKGRAY, 0.8f));
    
    DrawText("Minimal 3D Modeler", 10, 10, 20, WHITE);
    DrawText(TextFormat("FPS: %d", GetFPS()), GetScreenWidth() - 100, 10, 20, WHITE);
    
    // Left feature tree panel
    if (app.show_feature_tree) {
        int tree_width = 250;
        DrawRectangle(0, 40, tree_width, GetScreenHeight() - 40, ColorAlpha(LIGHTGRAY, 0.85f));
        DrawRectangleLines(0, 40, tree_width, GetScreenHeight() - 40, GRAY);
        
        DrawText("Scene Graph", 10, 50, 16, DARKGRAY);
        
        int y = 80;
        if (app.doc && app.doc->root) {
            DrawText("Root", 15, y, 14, BLACK);
            y += 25;
            
            for (int i = 0; i < app.doc->root->child_count; i++) {
                SceneNode* child = app.doc->root->children[i];
                if (child) {
                    const char* type_str = "Node";
                    if (child->type == NODE_FEATURE) type_str = "[Feature]";
                    else if (child->type == NODE_SKETCH) type_str = "[Sketch]";
                    
                    DrawText(TextFormat("  %s %s", type_str, child->name), 20, y, 12, DARKBLUE);
                    y += 20;
                    
                    if (y > GetScreenHeight() - 60) break;
                }
            }
        }
    }
    
    // Right property panel
    if (app.show_property_manager) {
        int prop_width = 280;
        int prop_x = GetScreenWidth() - prop_width;
        DrawRectangle(prop_x, 40, prop_width, GetScreenHeight() - 40, ColorAlpha(LIGHTGRAY, 0.85f));
        DrawRectangleLines(prop_x, 40, prop_width, GetScreenHeight() - 40, GRAY);
        
        DrawText("Properties", prop_x + 10, 50, 16, DARKGRAY);
        
        int y = 85;
        DrawText("Tool:", prop_x + 10, y, 12, DARKGRAY);
        y += 20;
        
        const char* tool_names[] = {"Select", "Sketch", "Extrude"};
        for (int i = 0; i < 3; i++) {
            Color color = (app.selected_tool == i) ? GREEN : LIGHTGRAY;
            DrawText(tool_names[i], prop_x + 20, y, 12, color);
            y += 20;
        }
        
        y += 10;
        DrawText("Render:", prop_x + 10, y, 12, DARKGRAY);
        y += 20;
        const char* render_modes[] = {"Standard 3D", "Raymarching"};
        for (int i = 0; i < 2; i++) {
            Color color = (app.use_raymarching == (i == 1)) ? GREEN : LIGHTGRAY;
            DrawText(render_modes[i], prop_x + 20, y, 12, color);
            y += 20;
        }
        
        y += 10;
        DrawText("Camera:", prop_x + 10, y, 12, DARKGRAY);
        y += 20;
        DrawText(TextFormat("Distance: %.2f", app.cam_distance), prop_x + 15, y, 11, BLACK);
        y += 18;
        DrawText(TextFormat("Yaw: %.2f", app.cam_yaw), prop_x + 15, y, 11, BLACK);
        y += 18;
        DrawText(TextFormat("Pitch: %.2f", app.cam_pitch), prop_x + 15, y, 11, BLACK);
    }
    
    // Status bar
    int status_y = GetScreenHeight() - 30;
    DrawRectangle(0, status_y, GetScreenWidth(), 30, ColorAlpha(DARKGRAY, 0.8f));
    DrawText("Right-drag: rotate  |  Scroll: zoom  |  H: toggle tree  |  P: toggle props  |  Shift+R: toggle raymarching", 10, status_y + 5, 14, WHITE);
}

// ============================================================================
// Main
// ============================================================================

int main(void)
{
    const int screen_width = 1280;
    const int screen_height = 720;
    
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(screen_width, screen_height, "Minimal 3D Modeler - SolidWorks-like CAD");
    SetTargetFPS(60);
    
    TraceLog(LOG_INFO, "Window initialized");
    
    // Initialize app state
    app.cam_yaw = -0.5f;
    app.cam_pitch = -0.3f;
    app.cam_distance = 7.0f;
    app.cam_target = (Vector3){0, 0, 0};
    app.show_property_manager = true;
    app.show_feature_tree = true;
    app.selected_tool = 0;
    app.use_raymarching = false;
    
    // Create document
    app.doc = scene_create_document("New Model");
    if (!app.doc) {
        TraceLog(LOG_ERROR, "Failed to create scene document");
        CloseWindow();
        return 1;
    }
    
    TraceLog(LOG_INFO, "Scene document created");
    
    // Create a test sketch
    SketchPlane xy_plane = {
        .normal = {0, 1, 0},
        .origin = {0, 0, 0}
    };
    Sketch* sketch = sketch_create("Sketch1", xy_plane);
    
    if (sketch) {
        sketch_add_line(sketch, (Vec2){-1, -1}, (Vec2){1, -1});
        sketch_add_line(sketch, (Vec2){1, -1}, (Vec2){1, 1});
        sketch_add_line(sketch, (Vec2){1, 1}, (Vec2){-1, 1});
        sketch_add_line(sketch, (Vec2){-1, 1}, (Vec2){-1, -1});
        sketch_solve(sketch);
        TraceLog(LOG_INFO, "Sketch created with 4 lines");
    }
    
    // Create extrude feature
    SceneNode* extrude_node = NULL;
    if (sketch) {
        extrude_node = extrude_create_feature(sketch, 2.0f, "Extrude1");
        if (extrude_node) {
            scene_add_child(app.doc->root, extrude_node);
            TraceLog(LOG_INFO, "Extrude feature created");
        }
    }
    
    // Create SDF tree
    SDFNode* sdf_root = sdf_create_sphere(1.0f);
    if (sdf_root) {
        sdf_set_transform(sdf_root, mat4_translate(0, 0, 0));
        TraceLog(LOG_INFO, "SDF sphere created");
    }
    
    // Initialize raymarching renderer
    app.renderer = raymarch_renderer_create();
    if (app.renderer && sdf_root) {
        raymarch_renderer_set_sdf_root(app.renderer, sdf_root);
        raymarch_renderer_set_resolution(app.renderer, screen_width - 530, screen_height - 70);
        TraceLog(LOG_INFO, "Raymarching renderer initialized");
    }
    
    // Main loop
    while (!WindowShouldClose()) {
        update_app_camera();
        
        if (app.doc) {
            scene_update(app.doc);
        }
        
        // Input handling
        if (IsKeyPressed(KEY_ONE)) app.selected_tool = 0;
        if (IsKeyPressed(KEY_TWO)) app.selected_tool = 1;
        if (IsKeyPressed(KEY_THREE)) app.selected_tool = 2;
        if (IsKeyPressed(KEY_H)) app.show_feature_tree = !app.show_feature_tree;
        if (IsKeyPressed(KEY_P)) app.show_property_manager = !app.show_property_manager;
        if (IsKeyPressed(KEY_R) && IsKeyDown(KEY_LEFT_SHIFT)) {
            app.use_raymarching = !app.use_raymarching;
            TraceLog(LOG_INFO, app.use_raymarching ? "Raymarching ENABLED" : "Raymarching DISABLED");
        }
        
        // Draw
        BeginDrawing();
        ClearBackground((Color){40, 40, 40, 255});
        
        if (app.use_raymarching && app.renderer && sdf_root) {
            raymarch_renderer_draw(app.renderer);
        } else {
            // Standard 3D rendering
            Camera3D cam = {
                .position = {app.cam_distance * cosf(app.cam_pitch) * cosf(app.cam_yaw),
                             app.cam_distance * sinf(app.cam_pitch),
                             app.cam_distance * cosf(app.cam_pitch) * sinf(app.cam_yaw)},
                .target = app.cam_target,
                .up = {0, 1, 0},
                .fovy = 60.0f,
                .projection = CAMERA_PERSPECTIVE
            };
            
            BeginMode3D(cam);
            
            // Reference geometry
            DrawGrid(10, 1.0f);
            DrawLine3D((Vector3){0, 0, 0}, (Vector3){5, 0, 0}, RED);
            DrawLine3D((Vector3){0, 0, 0}, (Vector3){0, 5, 0}, GREEN);
            DrawLine3D((Vector3){0, 0, 0}, (Vector3){0, 0, 5}, BLUE);
            
            // Test sphere
            DrawSphere((Vector3){0, 0, 0}, 1.0f, MAROON);
            
            EndMode3D();
        }
        
        draw_ui();
        EndDrawing();
    }
    
    // Cleanup
    if (app.renderer) raymarch_renderer_free(app.renderer);
    if (sdf_root) sdf_free_tree(sdf_root);
    if (sketch) sketch_free(sketch);
    if (app.doc) scene_free_document(app.doc);
    
    CloseWindow();
    return 0;
}
