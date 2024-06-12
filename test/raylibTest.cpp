#include <iostream>
#include "DrawInterface.h"
#include "InputManager.h"

using namespace std;
using namespace ReyEngine;

int main(int argc, char** argv){

   Size<int> windowSize(800, 600);
   InitWindow(windowSize.x, windowSize.y, "basic drawsing test");

   Camera2D camera = {};
   camera.rotation = 0;
   camera.offset.x = GetScreenWidth() / 2;
   camera.offset.y = GetScreenHeight() / 2;
   camera.target.x = camera.offset.x;
   camera.target.y = camera.offset.y;
   camera.zoom = 1.0f;
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
      if (mvVec) {
         auto newVec = mvVec * moveSpeed;
         camera.target.x += newVec.x;
         camera.target.y += newVec.y;
      }
      if (rotation){
         camera.rotation += rotation;
      }

      BeginDrawing();
      ClearBackground(WHITE);
      DrawText("Foreground", 300, 300, 20, RED);
      DrawRectangle(0,0,20,20,BLUE);
      BeginMode2D(camera);
      DrawText("Background", 100, 100, 20, BLACK);
      DrawRectangle(100,200,20,20,GREEN);
      EndMode2D();

      DrawText("MoreForeground", 300, 400, 20, RED);

      EndDrawing();

   }

   return 0;
}