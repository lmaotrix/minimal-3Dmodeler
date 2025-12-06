#include "raylib.h"
#include "rlgl.h"
#include <math.h>

typedef struct {
    Vector3 pos;
    Vector3 target;
    float yaw, pitch;
    float distance;
} OrbitCamera;

OrbitCamera cam;

void UpdateOrbitCamera(float dt) 
{
    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
    {
        Vector2 delta = GetMouseDelta();
        cam.yaw -= delta.x * 0.003f;
        cam.pitch -= delta.y * 0.003f;
        if (cam.pitch > 1.5f) cam.pitch = 1.5f;
        if (cam.pitch < -1.5f) cam.pitch = -1.5f;
    }

    cam.distance -= GetMouseWheelMove() * 0.5f;
    if (cam.distance < 1.0f) cam.distance = 1.0f;

    cam.pos.x = cam.target.x + cosf(cam.pitch) * cosf(cam.yaw) * cam.distance;
    cam.pos.y = cam.target.y + sinf(cam.pitch) * cam.distance;
    cam.pos.z = cam.target.z + cosf(cam.pitch) * sinf(cam.yaw) * cam.distance;
}



int main(void)
{

    int screenWidth = 1280;
    int screenHeight = 720;
    InitWindow(screenWidth, screenHeight, "Day 1, Ray Marching in C");
    SetTargetFPS(60);

    cam.pos = (Vector3){ 3.0f, 2.0f, 3.0f };
    cam.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    cam.yaw = -0.5f;
    cam.pitch = -0.3f;
    cam.distance = 5.0f;

    Shader sh = LoadShader(0, "raymarch.fs");
    sh.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(sh, "camPos");
    int camTargetLoc = GetShaderLocation(sh, "camTarget");
    int resolutionLoc = GetShaderLocation(sh, "resolution");
    int timeLoc = GetShaderLocation(sh, "uTime");
    
    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();
        UpdateOrbitCamera(dt);

        BeginDrawing();
        ClearBackground(BLACK);

        BeginShaderMode(sh);

        Vector2 res = { (float)screenWidth, (float)screenHeight };
        SetShaderValue(sh, resolutionLoc, &res, SHADER_UNIFORM_VEC2);
        SetShaderValue(sh, timeLoc, &(float){ (float)GetTime() }, SHADER_UNIFORM_FLOAT);

        SetShaderValue(sh, sh.locs[SHADER_LOC_VECTOR_VIEW], &cam.pos, SHADER_UNIFORM_VEC3);
        SetShaderValue(sh, camTargetLoc, &cam.target, SHADER_UNIFORM_VEC3);

        rlBegin(RL_QUADS);
        rlVertex2f(-1, -1);
        rlVertex2f(1, -1);
        rlVertex2f(1, 1);
        rlVertex2f(-1, 1);
        rlEnd();

        EndShaderMode();
        EndDrawing();
    }
    UnloadShader(sh);
    CloseWindow();
    return 0;
}