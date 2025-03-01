#include "Window2.h"
#include "Widget2.h"
#include "Canvas2.h"

using namespace std;
using namespace ReyEngine;
using namespace ReyEngine::Internal::Tree;

struct Label2 : public Widget2 {
  Label2(const std::string& text)
  : text(text)
  , color(Colors::randColor())
  {
     setSize({100,100});
  }
   void render2D() const override {
      drawRectangle(getRect().toSizeRect(), color);
      drawText(text, {0,0}, getDefaultFont());
      drawText(Pos<int>(getPosition()), {0,20}, getDefaultFont());
      drawText(localMousePos, {0,40}, getDefaultFont());
   }
protected:
   Handled _unhandled_input(const InputEvent& event) override {
     if (auto isMouse = event.isMouse()){
        cout << _node->name << " got unhandled input @ localpos " << isMouse.value()->getLocalPos() << endl;
        localMousePos = isMouse.value()->getLocalPos();
     }

     return false;
  }
  Pos<int> localMousePos;
  std::string text;
  ColorRGBA color;
};

int main(){
   //create window
   {
      auto& window = Application2::createWindowPrototype("window", 1920, 1080, {WindowFlags::RESIZE}, 60)->createWindow();
      auto root = window.getCanvas();

      auto makeName = []() -> std::string{
         return "Label" + to_string(Application2::generateRandom(0,200000));
      };
      std::vector<TypeNode*> labels;
      {
         auto [widget2, _] = make_node<Label2>("Parent", "Parent");
         labels.push_back(root->getNode()->addChild(std::move(_)));
         widget2->setPosition({50, 50});
      }
      {
         auto [widget3, _] = make_node<Label2>("Child", "child");
         labels.push_back(labels.at(0)->addChild(std::move(_)));
         widget3->setPosition({50, 50});
      }

      window.exec();
   }
   return 0;

}