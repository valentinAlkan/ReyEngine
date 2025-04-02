#include <iostream>
#include "ReyEngine.h"
#include "InputManager.h"
#include "rlgl.h"
#include "raymath.h"

using namespace std;
using namespace ReyEngine;

int main(){
   const int screenWidth = 3000;
   const int screenHeight = 1200;

   InitWindow(screenWidth, screenHeight, "raylib [core] example - basic window");
   SetTargetFPS(60);


   constexpr Size<float> targetSize = {800,600};
   constexpr Size<float> glassSize = {200,200};
   constexpr Rect<float> src = {0,0,targetSize.x, targetSize.y};
   constexpr Rect<float> viewport = targetSize.toRect({100,100});
   RenderTarget target(targetSize);
   auto font = getDefaultFont();
   while (!WindowShouldClose())
   {
      auto cursorPos = InputManager::getMousePos().get();
      auto glassRect = (cursorPos - viewport.pos()).toRect(glassSize).centerOnPoint(cursorPos - viewport.pos()).restrictTo(src);
      target.beginRenderMode();
      DrawRectangleGradientV(0, 0, targetSize.x, targetSize.y, RED, BLUE);
      drawText("Fuck all yall", {0,0}, font);
      drawRectangleLines(glassRect, 1.0, Colors::yellow);
      target.endRenderMode();

      BeginDrawing();
      ClearBackground(RAYWHITE);
      drawRenderTargetRect(target, src, viewport, Colors::none);
      drawRenderTargetRect(target, glassRect, viewport.copy().mirrorRight().pushX(4).setSize(glassSize), Colors::none);
      drawText(Pos<int>(cursorPos).toString(), cursorPos - Pos<float>(20,20), font);
      EndDrawing();
   }
   CloseWindow();
   return 0;
}