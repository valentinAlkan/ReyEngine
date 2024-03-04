#include <iostream>
#include "DrawInterface.h"

using namespace std;
using namespace ReyEngine;

int main(int argc, char** argv){

   Size<int> windowSize(800, 600);
   InitWindow(windowSize.x, windowSize.y, "basic drawsing test");

   RenderTarget targeta;
   RenderTarget targetb;

   targeta.setSize({windowSize.x/2, windowSize.y});
   targetb.setSize({windowSize.x/2, windowSize.y});

   while(!WindowShouldClose()){
      BeginDrawing();
//      {
//         targeta.beginRenderMode();
//         ClearBackground(ReyEngine::Colors::none);
//         DrawRectangle(0, 0, targeta.getSize().x, targeta.getSize().y, RED);
//         DrawText("Render Pass 1", 0, 0, 20, BLUE);
//         targeta.endRenderMode();
//         targeta.render({0, 0});
//      }
//      {
//         targetb.beginRenderMode();
//         ClearBackground(ReyEngine::Colors::none);
//         DrawRectangle(0, 0, targetb.getSize().x, targetb.getSize().y, BLUE);
//         DrawText("Render Pass 2!", 0, 0, 20, RED);
//         targetb.endRenderMode();
//         targetb.render({windowSize.x / 2, 0});
//      }
//
//      {
//         targeta.beginRenderMode();
//         DrawRectangle(0, 0, targeta.getSize().x, targeta.getSize().y, GREEN);
//         DrawText("Render Pass 3", 0, 0, 20, BLUE);
//         targeta.endRenderMode();
//         targeta.render({0, windowSize.y/2});
//      }
//
//      {
//         targetb.beginRenderMode();
//         DrawRectangle(0, 0, targetb.getSize().x, targetb.getSize().y, YELLOW);
//         DrawText("Render Pass 4", 0,0, 20, RED);
//         targetb.endRenderMode();
//         targetb.render({windowSize.x/2, windowSize.y/2});
//      }

      ClearBackground(GRAY);
      targeta.beginRenderMode();
      DrawRectangle(0,0,50,50, BLUE);
      targeta.endRenderMode();

      targetb.beginRenderMode();
      DrawRectangle(0,0,50,50, RED);
      targetb.endRenderMode();

      targeta.beginRenderMode();
      DrawRectangle(0,100,50,50, GREEN);
      targeta.endRenderMode();

      targetb.beginRenderMode();
      DrawRectangle(0,100,50,50, YELLOW);
      targetb.endRenderMode();

      //draw
      targeta.render({0, 0});
      targetb.render({200, 0});

      EndDrawing();
   }

   return 0;
}