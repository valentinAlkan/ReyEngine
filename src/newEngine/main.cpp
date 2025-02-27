#include "Window2.h"
#include "Widget2.h"
#include "Canvas2.h"

using namespace std;
using namespace ReyEngine;
using namespace ReyEngine::Internal::Tree;



struct Sprite : public Widget2 {
  void render2D() const override {
     drawText("here is some text", Pos<float>(0,0), getDefaultFont());
  }
};



int main(){
   //create window
   //createWindowPrototype(const std::string &title, int width, int height, const std::vector<ReyEngine::WindowFlags> &flags, int targetFPS) {
   {
      auto& window = Application2::createWindowPrototype("window", 1920, 1080, {WindowFlags::RESIZE}, 60)->createWindow();
      auto root = window.getCanvas();

      {
         auto widget = make_node<Sprite>("sprite");
         root->getNode()->addChild(std::move(widget));
      }
      window.exec();
   }
   return 0;

}