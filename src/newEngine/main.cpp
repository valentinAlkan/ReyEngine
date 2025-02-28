#include "Window2.h"
#include "Widget2.h"
#include "Canvas2.h"

using namespace std;
using namespace ReyEngine;
using namespace ReyEngine::Internal::Tree;



struct Label2 : public Widget2 {
  Label2(const std::string& text): text(text){}
   void render2D() const override {
     drawText(text, {0,0}, getDefaultFont());
     drawText(Pos<int>(getPosition()), {0,20}, getDefaultFont());
  }
  std::string text;
};



int main(){
   //create window
   //createWindowPrototype(const std::string &title, int width, int height, const std::vector<ReyEngine::WindowFlags> &flags, int targetFPS) {
   {
      auto& window = Application2::createWindowPrototype("window", 1920, 1080, {WindowFlags::RESIZE}, 60)->createWindow();
      auto root = window.getCanvas();

      {
         auto [widget, node] = make_node<Label2>("Label0", "Here is some text");
         root->getNode()->addChild(std::move(node));
         widget->setPosition({50,50});
      }
      window.exec();
   }
   return 0;

}