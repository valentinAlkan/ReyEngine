#include "ReyEngine.h"
#include "Application.h"
#include "Window.h"
#include "Widget.h"
#include "FileBrowser.h"
#include "Label.h"

using namespace ReyEngine;
using namespace std;

int main(){
   auto& window = Application::createWindowPrototype("window", 1920, 1080, {WindowFlags::RESIZE}, 60)->createWindow();
   auto root = window.getCanvas();
   auto scrollArea = make_child<ScrollArea>(root, "scrollArea");
   scrollArea->setAnchoring(ReyEngine::Anchor::FILL);

   static constexpr std::array<float, 12> FONT_SIZES = {96, 64, 48, 32, 24, 20, 16, 14, 12, 10, 8, 6};

   auto fontDir = (FileSystem::Directory(CrossPlatform::getUserDir()) / "fonts").toDir();
   if (!fontDir.isDirectory()){
      Logger::error() << "Directory " << fontDir.abs() << " does not exist!" << endl;
   }
   int nextPos = 0;
   Logger::info() << "Searching for .ttf fonts in " << fontDir.canonical() << endl;
   for (const auto& fontFileName : fontDir.listContents()){
      auto tail = fontFileName.tail();
      if (!tail.ends_with(".ttf")) continue;
      for (auto i : FONT_SIZES){
//         static constexpr float FONT_INCR = 10;
         auto fontSize = i ;
         auto label = make_child<Label>(scrollArea, tail + to_string(fontSize), tail);
         label->getTheme().font = make_shared<ReyEngineFont>(fontFileName.canonical(), fontSize);
         auto& font = label->getTheme().font;
         font->size = fontSize;
         label->setPosition(label->getPos() + Pos<float>(0, nextPos));
         nextPos += fontSize + 2;
         label->setText(tail + " : " + to_string((int)fontSize));
         Logger::info() << "Instantiating font " << fontFileName.tail() << " " << fontSize << " at " << label->getPos() << endl;
      }
   }

   Logger::info() << "Screen DPI is " << Vec2<float>(GetWindowScaleDPI()) << endl;
   window.exec();
   return 0;
}