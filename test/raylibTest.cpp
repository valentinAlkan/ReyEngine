#include <iostream>
#include "DrawInterface.h"

using namespace std;
using namespace ReyEngine;

int main(int argc, char** argv){

   Size<int> windowSize(800, 600);
   InitWindow(windowSize.x, windowSize.y, "basic drawsing test");

   while(!WindowShouldClose()){
      BeginDrawing();
      ClearBackground(GRAY);
      BeginScissorMode(0,0,500,500);
      DrawRectangle(0,0,9999,9999,RED);
      DrawText("InScissor", 0,0, 20, BLACK);
      BeginScissorMode(200,200,100,100);
      DrawRectangle(0,0,9999,9999,BLUE);
      BeginScissorMode(250,250,25,25);
      DrawRectangle(0,0,9999,9999,GREEN);
      EndScissorMode();
      DrawText("OutScissor", 200,200,100,PURPLE);
//      BeginScissorMode(0,0,0,0);
//      DrawText("you shouldn't see this", 0,0,100,YELLOW);
      EndScissorMode();
      EndDrawing();

   }

   return 0;
}