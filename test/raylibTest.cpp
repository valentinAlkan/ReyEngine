#include <iostream>
#include "ReyEngine.h"
#include "InputManager.h"
#include "rlgl.h"
#include "raymath.h"
#include "Platform.h"

using namespace std;
using namespace ReyEngine;
#define OLD 0
static constexpr std::array<float, 12> sizes = {96, 64, 48, 32, 24, 20, 16, 14, 12, 10, 8, 6};
int main(){
   const int screenWidth = 3000;
   const int screenHeight = 1200;

   InitWindow(screenWidth, screenHeight, "raylib [core] example - basic window");
   SetTargetFPS(60);

   auto fontFile = (FileSystem::Directory(CrossPlatform::getUserDir()) / "fonts" / "FiraCode-Medium.ttf").toFile();
   if (!fontFile.isDirectory()){
      Logger::error() << "File " << fontFile.abs() << " does not exist!" << endl;
   }
   Logger::info() << "using font file " << fontFile.canonical() << endl;

//   std::array<Font, sizes.size()> fonts = {};
   std::array<std::shared_ptr<ReyEngineFont>, sizes.size()> fonts;
   for (int i=0; i<fonts.size(); i++) {
      auto& font = fonts.at(i);
      auto size = sizes.at(i);
#if OLD
      font = LoadFontEx(fontFile.canonical().c_str(), size, 0, 0);
#else
      font = make_shared<ReyEngineFont>(fontFile.canonical(), size);
#endif
   }
   while (!WindowShouldClose())
   {
      BeginDrawing();
      ClearBackground(RAYWHITE);
      float nextPos = 0;
      for (auto font : fonts) {
         auto fontSize = font->size;
         DrawTextEx(font->font, to_string(fontSize).c_str(), {4, nextPos}, fontSize, 0, Colors::black);
         nextPos += fontSize;
      }
      EndDrawing();
   }
   CloseWindow();
   return 0;
}