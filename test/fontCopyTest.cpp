// Checks copying, moving and unloading ReyEngineFont. Needs a window, for the GPU. Run it under AddressSanitizer to catch
// a font being freed twice, used after it's freed, or leaked. Usage: FontCopyTest <path to a .ttf>
#include <cstdio>
#include <optional>
#include <string>
#include <utility>
#include "ReyEngine.h"

using namespace ReyEngine;

namespace {
   int failures = 0;
   void check(bool ok, const char* what){
      if (!ok) {std::printf("FAILED: %s\n", what); failures++;}
   }
   /// uses the font's glyphs and texture, so it's caught if they've been freed
   void use(const ReyEngineFont& font){
      BeginDrawing();
      drawText("Spring 6000 BC", Pos<float>(10, 10), font);
      EndDrawing();
      check(font.measure("Spring 6000 BC").x > 0, "the font still measures text");
   }
   bool sameLook(const ReyEngineFont& a, const ReyEngineFont& b){
      return a.size == b.size && a.spacing == b.spacing && a.color.r == b.color.r && a.colorDisabled.g == b.colorDisabled.g && a.fileName == b.fileName;
   }
}

int main(int argc, char** argv){
   if (argc != 2) {std::printf("usage: FontCopyTest <path to a .ttf>\n"); return 2;}
   const std::string path = argv[1];
   std::setvbuf(stdout, nullptr, _IONBF, 0); //a leak report ends the run before anything buffered is written
   InitWindow(200, 100, "font copy test");

   {
   ReyEngineFont original(path, 24);
   original.color = ColorRGBA(10, 20, 30);
   original.colorDisabled = ColorRGBA(40, 50, 60);
   original.spacing = 3;
   check(!original.isDefault, "the file loaded");

   { //a copy looks the same, and takes nothing with it when it goes
      ReyEngineFont copy(original);
      check(sameLook(copy, original), "a copy has the same size, spacing, colors and file");
      check(copy.font.texture.id == original.font.texture.id, "a copy shares the loaded font");
      copy.color = ColorRGBA(200, 0, 0);
      check(original.color.r == 10, "a copy's color is its own");
      use(copy);
   }
   use(original);

   { //the last one left unloads it, whichever that is
      auto first = std::make_optional<ReyEngineFont>(path, 18);
      ReyEngineFont second(*first);
      first.reset();
      use(second);
   }

   { //assigning over a font lets go of what it had
      ReyEngineFont other(path, 16);
      other = original;
      check(sameLook(other, original), "an assigned font looks the same");
      use(other);
   }
   use(original);

   { //a font can be assigned to itself
      ReyEngineFont self(path, 16);
      auto& same = self;
      self = same;
      use(self);
      self = std::move(same);
      use(self);
   }

   { //moving hands it over, and the font left behind doesn't unload it
      ReyEngineFont from(path, 20);
      from.color = ColorRGBA(70, 80, 90);
      ReyEngineFont to(std::move(from));
      check(to.size == 20 && to.color.r == 70, "a font moved from keeps its look in the new one");
      use(to);
      ReyEngineFont over(path, 14);
      over = std::move(to);
      check(over.size == 20, "a font moved over takes the new look");
      use(over);
   }

   { //the engine's own font is never unloaded
      {
         ReyEngineFont builtIn(30);
         ReyEngineFont copy(builtIn);
         check(copy.isDefault && copy.size == 30, "a copy of the built-in font is too");
      }
      ReyEngineFont again;
      use(again);
   }

   { //a font set up by hand is owned the same way
      ReyEngineFont byHand(20);
      byHand.font = LoadFontEx(path.c_str(), 20, nullptr, 0);
      byHand.isDefault = false;
      use(byHand);
   }

   }

   //one outliving the window just goes quietly: it isn't unloaded, so this one's the only font a leak check should report
   auto* outlives = new ReyEngineFont(path, 12);
   CloseWindow();
   delete outlives;

   if (failures) {std::printf("%d failures\n", failures); return 1;}
   std::printf("ok\n");
   return 0;
}
