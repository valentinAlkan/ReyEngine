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

      TypeNode* label0;
      TypeNode* label1;
      TypeNode* label2;
      TypeNode* label10;
      {
         auto [widget, _] = make_node<Label2>("Label0", "Parent");
         label0 = root->getNode()->addChild(std::move(_));
         widget->setPosition({50,50});
      }
      {
         auto [widget, _] = make_node<Label2>("Label1", "Child");
         label1 = label0->addChild(std::move(_));
         widget->setPosition({50,50});
      }
      {
         auto [widget, _] = make_node<Label2>("Label2", "Child");
         label2 = label1->addChild(std::move(_));
         widget->setPosition({50,50});
      }
      {
         auto [widget, _] = make_node<Label2>("Label10", "Child");
         label10 = label1->addChild(std::move(_));
         widget->setPosition({50,50});
      }
      window.exec();
   }
   return 0;

}