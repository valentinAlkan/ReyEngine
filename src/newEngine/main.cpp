#include "Window.h"
#include "Widget.h"
#include "Canvas.h"

using namespace std;
using namespace ReyEngine;
using namespace ReyEngine::Internal::Tree;

struct Label2 : public Widget {
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
        localMousePos = isMouse.value()->getLocalPos();
        isInside = isMouse.value()->isInside();
        switch (event.eventId) {
           case InputEventMouseButton::ID:{
              auto& mbEvent = event.toEvent<InputEventMouseButton>();
              if (isInside && mbEvent.isDown) {
                 isDown = true;
                 return true;
              }
              if (isDown && !mbEvent.isDown){
                 isDown = false;
                 return true;
              }
              break;}
           case InputEventMouseMotion::ID:{
              auto& mmEvent = event.toEvent<InputEventMouseMotion>();
              if (isDown){
                 setPosition(getPosition() + mmEvent.mouseDelta);
                 cout << "setting new position " << getPosition() << endl;
              }
              break;}
        }
     }

     return false;
  }
  Pos<int> localMousePos;
  bool isInside = false;
  std::string text;
  ColorRGBA color;
  bool isDown = false;
};

int main(){
   //create window
   {
      auto& window = Application::createWindowPrototype("window", 1920, 1080, {WindowFlags::RESIZE}, 60)->createWindow();
      auto root = window.getCanvas();

      auto makeName = []() -> std::string{
         return "Label" + to_string(Application::generateRandom(0, 200000));
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