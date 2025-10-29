#include <iostream>
#include "ReyEngine.h"
#include "InputManager.h"
#include "rlgl.h"
#include "raymath.h"
#include "Platform.h"

using namespace std;
using namespace ReyEngine;
#define OLD 0
int main(){
   const int screenWidth = 800;
   const int screenHeight = 600;

   InitWindow(screenWidth, screenHeight, "raylib test");
   SetTargetFPS(60);

   ReyEngine::FileSystem::File file = "/root/containerJunk/markingtest/markingGuiTestFile.mp4";
   if (!file.exists()) throw std::runtime_error("File "+ file.abs() + "does not exist!");

   std::stringstream cmd;
   const int frameIndex = 0;
   cmd << "ffmpeg -i \"" << file.canonical() << "\" -vf \"select=eq(n\\," << frameIndex << ")\" -vframes 1 -f image2pipe -vcodec png - 2>/dev/null";


   cout << cmd.str() << endl;

   FILE* pipe = popen(cmd.str().c_str(), "r");
   if (!pipe) throw std::runtime_error("Pipe error!");

   // Read PNG data from pipe
   std::vector<uint8_t> imageData;
   char buffer[32768];
   size_t bytesRead;

   while ((bytesRead = fread(buffer, 1, sizeof(buffer), pipe)) > 0) {
      imageData.insert(imageData.end(), buffer, buffer + bytesRead);
   }

   if (auto retval = pclose(pipe)){
      throw std::runtime_error("Invalid pclose return value: " + std::to_string(retval));
   }
   if (imageData.empty()) throw std::runtime_error("Pipe error!");
   // Load image from memory
   std::shared_ptr<ReyEngine::ReyTexture> _tex = std::make_shared<ReyEngine::ReyTexture>(LoadImageFromMemory(".png", imageData.data(), (int)imageData.size()));

   while (!WindowShouldClose())
   {
      BeginDrawing();
      ClearBackground(RAYWHITE);

      DrawTexture(_tex->getTexture(), 0, 0, RAYWHITE);


      EndDrawing();
   }
   CloseWindow();
   return 0;
}