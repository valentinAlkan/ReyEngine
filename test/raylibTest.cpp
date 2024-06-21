#include <iostream>
#include "DrawInterface.h"
#include "InputManager.h"
#include "rlgl.h"
#include <stack>

using namespace std;
using namespace ReyEngine;

bool cameraToggle = false;

int main(int argc, char** argv){
   Camera2D foregroundCamera = {0}; //BEGIN to draw background
   foregroundCamera.zoom = 1.0;
   Size<int> windowSize(800, 600);
   InitWindow(windowSize.x, windowSize.y, "basic drawsing test");
   SetTargetFPS(60);


   // Define the camera to look into our 3d world
   Camera3D camera = { 0 };
   camera.position = (Vector3){ 10.0f, 10.0f, 10.0f }; // Camera position
   camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };      // Camera looking at point
   camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
   camera.fovy = 45.0f;                                // Camera field-of-view Y
   camera.projection = CAMERA_PERSPECTIVE;             // Camera projection type

   Vector3 cubePosition = { 0.0f, 0.0f, 0.0f };
   RenderTexture2D renderTarget = LoadRenderTexture(windowSize.x, windowSize.y);

   while(!WindowShouldClose()){
      int moveSpeed = 5;
      Vec2<int> mvVec;
      double rotation = 0;
      if (InputManager::isKeyDown(InputInterface::KeyCodes::KEY_W)){
         mvVec += {0, -1};
      }
      if (InputManager::isKeyDown(InputInterface::KeyCodes::KEY_A)) {
         mvVec += {-1, 0};
      }
      if (InputManager::isKeyDown(InputInterface::KeyCodes::KEY_S)) {
         mvVec += {0, 1};
      }
      if (InputManager::isKeyDown(InputInterface::KeyCodes::KEY_D)) {
         mvVec += {1, 0};
      }
      if (InputManager::isKeyDown(InputInterface::KeyCodes::KEY_Q)) {
         rotation += 1;
      }
      if (InputManager::isKeyDown(InputInterface::KeyCodes::KEY_E)) {
         rotation -= 1;
      }
      if (InputManager::isMousButtonPressed(InputInterface::MouseButton::LEFT)){
         cameraToggle = !cameraToggle;
         std::cout << "Switching camera!" << endl;
      }

      if (mvVec) {
         auto newVec = mvVec * moveSpeed;
         foregroundCamera.offset += newVec;
         cout << Vec2<float>(foregroundCamera.offset) << endl;
      }
      if (rotation){
         foregroundCamera.rotation += rotation;
      }

//      2d camera
//      BeginDrawing();
//      ClearBackground(WHITE);
//      // Draw somebackground
//      BeginMode2D(foregroundCamera);
//      {
//         DrawText("CameraBackground", 100, 100, 20, RED);
//         DrawRectangle(100, 200, 20, 20, RED);
//      }
//      EndMode2D();
//
//
//      DrawText("Foreground", 300, 300, 20, BLUE);
//      DrawRectangle(0, 0, 20, 20, BLUE);
//
//      //Draw more background
//      BeginMode2D(foregroundCamera);
//      {
//         DrawText("More background", 500, 300, 20, RED);
//         DrawRectangle(40, 20, 20, 20, RED);
//      }
//      EndMode2D();
//      EndDrawing();

      // Draw somebackground

//      Rendering3D to a texture
      BeginTextureMode(renderTarget);              // Begin drawing to render texture
      ClearBackground(WHITE);
      BeginMode3D(camera);


      DrawCube(cubePosition, 2.0f, 2.0f, 2.0f, RED);
      DrawCubeWires(cubePosition, 2.0f, 2.0f, 2.0f, MAROON);

      DrawGrid(10, 1.0f);

      EndMode3D();
      EndTextureMode();

      BeginDrawing();
      ClearBackground(WHITE);
      //DRaw the texture
      DrawTextureRec(renderTarget.texture, {0,0,(float)windowSize.x, -(float)windowSize.y}, {0,0}, WHITE);

      EndDrawing();
   }

   return 0;
}