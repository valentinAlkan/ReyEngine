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

      BeginDrawing();
      ClearBackground(WHITE);
      // Draw somebackground
      BeginMode2D(foregroundCamera);
      {
         DrawText("CameraBackground", 100, 100, 20, RED);
         DrawRectangle(100, 200, 20, 20, RED);
      }
      EndMode2D();


      DrawText("Foreground", 300, 300, 20, BLUE);
      DrawRectangle(0, 0, 20, 20, BLUE);

      //Draw more background
      BeginMode2D(foregroundCamera);
      {
         DrawText("More background", 500, 300, 20, RED);
         DrawRectangle(40, 20, 20, 20, RED);
      }
      EndMode2D();
      EndDrawing();

   }

   return 0;
}