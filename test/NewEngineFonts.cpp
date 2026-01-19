#include "ReyEngine.h"
#include "Application.h"
#include "Window.h"
#include "Widget.h"
#include "FileBrowser.h"
#include "Label.h"
#include "ArgParse.h"

using namespace ReyEngine;
using namespace std;

int main(int argc, char** argv){
   ArgParse args;
   args.defineArg(RuntimeArg("--fontPath", "fontPath", 0, RuntimeArg::ArgType::FLAG));
   args.parseArgs(argc, argv);

   auto& window = Application::createWindowPrototype("window", 1920, 1080, {WindowFlags::RESIZE}, 60)->createWindow();
   auto root = window.getCanvas();
   auto scrollArea = make_child<ScrollArea>(root, "scrollArea");
   scrollArea->setAnchoring(ReyEngine::Anchor::FILL);

   static constexpr std::array<float, 12> FONT_SIZES = {96, 64, 48, 32, 24, 20, 16, 14, 12, 10, 8, 6};

   auto fontDir = FileSystem::Directory(CrossPlatform::getFontsDir());
   if (auto path = args.getArg("fontPath")){
      fontDir = FileSystem::Directory(path.value()->getParams().at(0));
   }

   if (!fontDir.exists()){
      Logger::error() << "Directory " << fontDir.abs() << " does not exist!" << endl;
      exit(1);
   }

   int nextPos = 0;
   Logger::info() << "Searching for .ttf fonts in " << fontDir.canonical() << endl;
   for (const auto& fontFileName : fontDir.listContents().first){
      auto tail = fontFileName.tail();
      if (!tail.ends_with(".ttf")) continue;
      for (auto i : FONT_SIZES){
         auto fontSize = i ;
         auto label = make_child<Label>(scrollArea, tail + to_string(fontSize), tail);
         label->setTheme(make_shared<Theme>());
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