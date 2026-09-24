#include "raylib.h"
#include "../core/scene.h"
#include "../renderer/sdf.h"
#include "../renderer/raymarch_renderer.h"

typedef struct {
    SceneDocument* doc;
    RaymarchRenderer* renderer;
    
    // Camera
    Vector3 cam_pos;
    Vector3 cam_target;
    float cam_yaw;
    float cam_pitch;
    float cam_distance;
    
    // UI state
    bool show_property_manager;
    bool show_feature_tree;
    bool show_grid;
    bool show_axes;
    
    // Interaction state
    bool is_dragging_camera;
    Vector2 mouse_start_pos;
    
    // Display settings
    DisplayMode display_mode;
    bool vsync_enabled;
} Application;

static Application app = {0};

// Camera control functions
static void UpdateCamera(float dt) {
    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
        if (!app.is_dragging_camera) {
            app.is_dragging_camera = true;
            app.mouse_start_pos = GetMousePosition();
        }
        
        Vector2 delta = GetMouseDelta();
        app.cam_yaw -= delta.x * 0.003f;
        app.cam_pitch -= delta.y * 0.003f;
        
        // Limit pitch
        if (app.cam_pitch > 1.5f) app.cam_pitch = 1.5f;
        if (app.cam_pitch < -1.5f) app.cam_pitch = -1.5f;
    } else {
        app.is_dragging_camera = false;
    }
    
    // Zoom
    app.cam_distance -= GetMouseWheelMove() * 0.5f;
    if (app.cam_distance < 1.0f) app.cam_distance = 1.0f;
    
    // Update camera position
    app.cam_pos.x = app.cam_target.x + cosf(app.cam_pitch) * cosf(app.cam_yaw) * app.cam_distance;
    app.cam_pos.y = app.cam_target.y + sinf(app.cam_pitch) * app.cam_distance;
    app.cam_pos.z = app.cam_target.z + cosf(app.cam_pitch) * sinf(app.cam_yaw) * app.cam_distance;
}

static void DrawGrid() {
    if (!app.show_grid) return;
    
    const float grid_size = 10.0f;
    const float grid_step = 1.0f;
    
    rlPushMatrix();
    rlTranslatef(0, -0.01f, 0);  // Slightly below origin to avoid z-fighting
    
    // Draw grid lines
    for (float x = -grid_size; x <= grid_size; x += grid_step) {
        Color color = (fabsf(x) < 0.01f) ? RED : DARKGRAY;
        DrawLine3D((Vector3){x, 0, -grid_size}, (Vector3){x, 0, grid_size}, color);
    }
    
    for (float z = -grid_size; z <= grid_size; z += grid_step) {
        Color color = (fabsf(z) < 0.01f) ? BLUE : DARKGRAY;
        DrawLine3D((Vector3){-grid_size, 0, z}, (Vector3){grid_size, 0, z}, color);
    }
    
    rlPopMatrix();
}

static void DrawAxes() {
    if (!app.show_axes) return;
    
    const float axis_length = 2.0f;
    
    DrawLine3D((Vector3){0, 0, 0}, (Vector3){axis_length, 0, 0}, RED);   // X
    DrawLine3D((Vector3){0, 0, 0}, (Vector3){0, axis_length, 0}, GREEN); // Y
    DrawLine3D((Vector3){0, 0, 0}, (Vector3){0, 0, axis_length}, BLUE);  // Z
    
    // Add axis labels
    DrawText3D("X", (Vector3){axis_length + 0.1f, 0, 0}, 0.1f, RED);
    DrawText3D("Y", (Vector3){0, axis_length + 0.1f, 0}, 0.1f, GREEN);
    DrawText3D("Z", (Vector3){0, 0, axis_length + 0.1f}, 0.1f, BLUE);
}

// Placeholder for DrawText3D (would need to be implemented properly)
static void DrawText3D(const char* text, Vector3 position, float fontSize, Color color) {
    // Simple 3D text drawing (this is a placeholder)
    // In a real implementation, we'd need to use billboarding or render to texture
    Vector2 screenPos = GetWorldToScreen(position, GetCamera());
    DrawText(text, (int)screenPos.x, (int)screenPos.y, (int)(fontSize * 10), color);
}

static void DrawUI() {
    // Top toolbar
    DrawRectangle(0, 0, GetScreenWidth(), 40, ColorAlpha(DARKGRAY, 0.8f));
    
    // Feature tree (left panel)
    if (app.show_feature_tree) {
        int tree_width = 300;
        DrawRectangle(0, 40, tree_width, GetScreenHeight() - 40, ColorAlpha(LIGHTGRAY, 0.9f));
        DrawRectangleLines(0, 40, tree_width, GetScreenHeight() - 40, GRAY);
        
        // Feature tree title
        DrawText("Feature Manager", 10, 50, 20, DARKGRAY);
    }
    
    // Property manager (right panel)
    if (app.show_property_manager) {
        int prop_width = 350;
        int prop_x = GetScreenWidth() - prop_width;
        DrawRectangle(prop_x, 40, prop_width, GetScreenHeight() - 40, ColorAlpha(LIGHTGRAY, 0.9f));
        DrawRectangleLines(prop_x, 40, prop_width, GetScreenHeight() - 40, GRAY);
        
        // Property manager title
        DrawText("Property Manager", prop_x + 10, 50, 20, DARKGRAY);
    }
    
    // Status bar
    int status_y = GetScreenHeight() - 30;
    DrawRectangle(0, status_y, GetScreenWidth(), 30, ColorAlpha(DARKGRAY, 0.8f));
    DrawText("Ready", 10, status_y + 5, 20, WHITE);
    
    // Display mode in status bar
    const char* mode_str = "Unknown";
    switch (app.display_mode) {
        case DISPLAY_SHADED: mode_str = "Shaded"; break;
        case DISPLAY_WIREFRAME: mode_str = "Wireframe"; break;
        case DISPLAY_SHADED_WITH_EDGES: mode_str = "Shaded with Edges"; break;
        case DISPLAY_HIDDEN_LINES_REMOVED: mode_str = "Hidden Lines Removed"; break;
    }
    DrawText(TextFormat("Mode: %s", mode_str), GetScreenWidth() - 200, status_y + 5, 20, WHITE);
}

int main(void) {
    const int screen_width = 1600;
    const int screen_height = 900;
    
    // Initialize window
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(screen_width, screen_height, "Minimal 3D Modeler - SolidWorks-like CAD");
    SetTargetFPS(app.vsync_enabled ? 60 : 120);
    
    // Initialize application state
    app.cam_pos = (Vector3){ 5.0f, 3.0f, 5.0f };
    app.cam_target = (Vector3){ 0.0f, 0.0f, 0.0f };
    app.cam_yaw = -0.5f;
    app.cam_pitch = -0.3f;
    app.cam_distance = 7.0f;
    
    app.show_property_manager = true;
    app.show_feature_tree = true;
    app.show_grid = true;
    app.show_axes = true;
    app.display_mode = DISPLAY_SHADED;
    app.vsync_enabled = true;
    
    // Create a new document
    app.doc = scene_create_document("New Document");
    
    // Create simple test scene
    SDFNode* sphere = sdf_create_sphere(1.0f);
    SDFNode* box = sdf_create_box((Vec3){1.5f, 1.0f, 1.0f});
    sdf_set_transform(box, mat4_translate(2.0f, 0.5f, 0));
    
    // Create a simple CSG object
    SDFNode* csg = sdf_subtract(box, sphere);
    
    // Initialize the raymarching renderer
    app.renderer = raymarch_renderer_create();
    if (app.renderer) {
        raymarch_renderer_set_sdf_root(app.renderer, csg);
    }
    
    // Main loop
    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        
        // Update
        UpdateCamera(dt);
        
        // Update renderer camera
        if (app.renderer) {
            Vec3 cam_pos_vec = {app.cam_pos.x, app.cam_pos.y, app.cam_pos.z};
            Vec3 cam_target_vec = {app.cam_target.x, app.cam_target.y, app.cam_target.z};
            raymarch_renderer_set_camera(app.renderer, cam_pos_vec, cam_target_vec);
        }
        
        // Draw
        BeginDrawing();
        ClearBackground(ColorAlpha(BLACK, 0.1f));
        
        // Draw raymarched scene
        if (app.renderer && app.display_mode == DISPLAY_SHADED) {
            raymarch_renderer_draw(app.renderer);
        }
        
        // Draw 3D helpers (grid, axes)
        DrawGrid();
        DrawAxes();
        
        // Draw UI
        DrawUI();
        
        // Draw FPS
        DrawFPS(10, 10);
        
        EndDrawing();
    }
    
    // Cleanup
    if (app.renderer) raymarch_renderer_free(app.renderer);
    if (csg) sdf_free_tree(csg);
    if (app.doc) scene_free_document(app.doc);
    
    CloseWindow();
    return 0;
}
