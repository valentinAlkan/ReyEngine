#include <iostream>
#include "DrawInterface.h"
#include "InputManager.h"
#include "rlgl.h"
#include <stack>

using namespace std;
using namespace ReyEngine;

bool cameraToggle = false;
struct CamTransform{
   Vec2<int> target;
   Vec2<int> position;
   double rotation;
};

std::stack<CamTransform> cameraStack;
int main(int argc, char** argv){

   Size<int> windowSize(800, 600);
   InitWindow(windowSize.x, windowSize.y, "basic drawsing test");
   SetTargetFPS(60);
   cameraStack.push(CamTransform());
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
         cameraStack.top().target += newVec;
      }
      if (rotation){
         cameraStack.top().rotation += rotation;
      }

      BeginDrawing();
      ClearBackground(WHITE);
      rlPushMatrix();
      rlTranslatef(-cameraStack.top().target.x, -cameraStack.top().target.y, 0);
      {

         DrawText("CameraBackground", 100, 100, 20, cameraToggle ? BLUE : LIGHTGRAY);
         DrawRectangle(100, 200, 20, 20, GREEN);

         rlPopMatrix();
            // Draw camera foreground
            DrawText("Foreground", 300, 300, 20, RED);
            DrawRectangle(0, 0, 20, 20, BLUE);
         rlPushMatrix();
         rlTranslatef(-cameraStack.top().target.x, -cameraStack.top().target.y, 0);

         DrawText("More background", 500, 300, 20, RED);
         DrawRectangle(40, 20, 20, 20, PURPLE);
      }
      rlPopMatrix();
      EndDrawing();

   }

   return 0;
}