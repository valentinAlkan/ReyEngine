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

   int pos = 0;
   for (const auto& fontFileName : FileSystem::Directory("/root/casper/ReyEngine/assets/fonts").listContents()){
      auto tail = fontFileName.tail();
      for (int i=0; i<6; i++){
         static constexpr float FONT_INCR = 5;
         auto fontSize = (i + 1) * FONT_INCR;
         auto label = make_child<Label>(root, tail + to_string(fontSize), tail);
         label->getTheme().font = ReyEngineFont(fontFileName.abs());
         auto& font = label->getTheme().font;
         font.size = fontSize;
         auto textSize = measureText(label->getText(), font);
         pos += textSize.y;
         label->setPosition(label->getPos() + Pos<float>(0, pos));
         label->setText(tail + " : " + to_string((int)textSize.y));
      }
   }

   window.exec();
   return 0;
}