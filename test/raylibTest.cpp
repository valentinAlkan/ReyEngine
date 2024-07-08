#include <iostream>
//#include "DrawInterface.h"
#include "InputManager.h"
#include "rlgl.h"
#include <stack>
//#include <Eigen/Dense>

using namespace std;
using namespace ReyEngine;

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
   RenderTexture2D canvasTarget = LoadRenderTexture(windowSize.x, windowSize.y);

   while(!WindowShouldClose()) {
      int moveSpeed = 5;
      Vec2<int> mvVec;
      double rotation = 0;
      if (InputManager::isKeyDown(InputInterface::KeyCodes::KEY_W)) {
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
      if (InputManager::isMouseButtonDown(InputInterface::MouseButton::LEFT)) {
         //see what the mouse delta is
         auto delta = InputManager::getMouseDelta();
         // Define the camera's initial position
//         Eigen::Vector3d cameraPosition(camera.position.x, camera.position.y, camera.position.z);
//
//         // Define the angle of rotation in degrees and convert it to radians
//         double angleDegrees = delta.x;
//         double angleRadians = angleDegrees * M_PI / 180.0;
//
//         // Define the rotation matrix around the y-axis
//         Eigen::Matrix3d rotationMatrix;
//         rotationMatrix = Eigen::AngleAxisd(angleRadians, Eigen::Vector3d::UnitY());
//
//         // Apply the rotation to the camera position
//         Eigen::Vector3d rotatedPosition = rotationMatrix * cameraPosition;
//
//         // Output the result
//         std::cout << "Original position: " << cameraPosition.transpose() << std::endl;
//         std::cout << "Rotated position: " << rotatedPosition.transpose() << std::endl;
//         camera.position.x = rotatedPosition.x();
//         camera.position.y = rotatedPosition.y();
//         camera.position.z = rotatedPosition.z();
      }

      if (mvVec) {
         auto newVec = mvVec * moveSpeed;
         foregroundCamera.offset += newVec;
         cout << Vec2<float>(foregroundCamera.offset) << endl;
      }
      if (rotation) {
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

      // Rendering3D to a (nested) texture
      BeginTextureMode(canvasTarget);              // Begin drawing to render texture
      {
         ClearBackground(WHITE);
         DrawText("This is the Camera UI", 0, windowSize.y - 20, 20, BLACK);

         {
            BeginMode2D(foregroundCamera);
            DrawText("This is the 2D background", 0, 0, 20, BLACK);

            {
               BeginMode3D(camera);
               DrawCube(cubePosition, 2.0f, 2.0f, 2.0f, RED);
               DrawSphere({1, 1, 1}, 0.5, BLUE);
               DrawCubeWires(cubePosition, 2.0f, 2.0f, 2.0f, MAROON);
               DrawGrid(10, 1.0f);
               EndMode3D();
            }

            EndMode2D();
         }
         EndTextureMode();
      }
      //window
      BeginDrawing();
      ClearBackground(WHITE);
      //DRaw the texture
      DrawTextureRec(canvasTarget.texture, {0, 0, (float)windowSize.x, -(float)windowSize.y}, {0, 0}, WHITE);
      EndDrawing();
      //end window
   }

   return 0;
}