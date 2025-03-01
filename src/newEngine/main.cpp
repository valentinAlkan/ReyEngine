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
     setSize({200,200});
  }
   void render2D() const override {
      drawRectangle(getRect().toSizeRect(), isInside ? color : Colors::lightGray);
      drawText(text, {0,0}, getDefaultFont());
      drawText("P = " + Pos<int>(getPosition()).toString(), {0,20}, getDefaultFont());
      drawText("S = " + Size<int>(getSize()).toString(), {0,40}, getDefaultFont());
      drawText(localMousePos, {0,60}, getDefaultFont());
   }
protected:
   Handled _unhandled_input(const InputEvent& event) override {
     if (auto isMouse = event.isMouse()){
        cout << _node->name << " got unhandled input @ localpos " << isMouse.value()->getLocalPos() << endl;
        localMousePos = isMouse.value()->getLocalPos();
        isInside = event.isMouse().value()->isInside();
     }

     return false;
  }
  Pos<int> localMousePos;
  bool isInside = false;
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
         widget2->setPosition({0, 00});
      }
      {
         auto [widget3, _] = make_node<Label2>("Child", "child");
         labels.push_back(labels.at(0)->addChild(std::move(_)));
         widget3->setPosition({150, 150});
      }

      window.exec();
   }
   return 0;

}