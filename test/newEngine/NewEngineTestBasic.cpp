#include "ReyEngine.h"
#include "Logger.h"
#include "Application.h"
#include "Window.h"
#include "Widget.h"

using namespace ReyEngine;
using namespace std;


struct TestWidget : public Widget {
   REYENGINE_OBJECT(TestWidget)

   TestWidget(const std::string& text)
   : text(text), color(Colors::randColor()) {
      setSize({200, 200});
   }

   void render2D() const override {
      bool _isInsideAtRenderTime = getLocalMousePos().isInside(getSizeRect());
      drawRectangle(getSizeRect(), _isInsideAtRenderTime ? color : Colors::lightGray);
      drawText(text, {0, 0}, getDefaultFont());
      drawText("Pos = " + Pos<int>(getPos()).toString(), {0, 20}, getDefaultFont());
      drawText("Siz = " + Size<int>(getSize()).toString(), {0, 40}, getDefaultFont());
      drawText("G" + globalMousePos.toString(), {0, 60}, getDefaultFont());
      drawText("L" + localMousePos.toString(), {0, 80}, getDefaultFont());
   }

protected:
   void _on_rect_changed() override {}

   Widget* _unhandled_input(const InputEvent& event) override {
      auto mousePos = InputManager::getMousePos().get();
      if (auto isMouse = event.isMouse()) {
         isInside = isMouse.value()->isInside();
         switch (event.eventId) {
            case InputEventMouseButton::ID: {
               auto& mbEvent = event.toEvent<InputEventMouseButton>();
               if (isInside && mbEvent.isDown) {
                  isDown = true;
                  return this;
               }
               if (isDown && !mbEvent.isDown) {
                  isDown = false;
                  return this;
               }
               break;
            }
            case InputEventMouseMotion::ID: {
               globalMousePos = isMouse.value()->getCanvasPos().get();
               localMousePos = isMouse.value()->getLocalPos();
               auto& mmEvent = event.toEvent<InputEventMouseMotion>();
               if (isDown) {
//                 cout << "----------------------" << endl;
//                 cout << getName() << endl;
//                 cout << "Mouse delta = " << mmEvent.mouseDelta << endl;
//                 cout << "current position = " << getPos() << endl;
//                 cout << "new position = " << mmEvent.mouseDelta + getPos() << endl;
                  setPosition(getPos() + mmEvent.mouseDelta.toPos());
                  return this;
               }
               break;
            }
         }
      }

      return nullptr;
   }

   Pos<int> localMousePos;
   Pos<int> globalMousePos;
   bool isInside = false;
   std::string text;
   ColorRGBA color;
   bool isDown = false;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct ScissorWidget : public TestWidget {
   REYENGINE_OBJECT(ScissorWidget)
   ScissorWidget(const std::string& text) : TestWidget(text){
      setSize({200,200});
      scissorRect = getSizeRect();
   }
   void render2D() const override {
      ScopeScissor scissor(getGlobalTransform(), scissorRect);
      TestWidget::render2D();
      cout << scissor.getRect() << endl;
   }
   void setScissorArea(const Rect<float>& r){
      scissorRect = r;
   }
   Rect<float> scissorRect;
};

int main(){
   auto& window = Application::createWindowPrototype("window", 1920, 1080, {WindowFlags::RESIZE}, 60)->createWindow();
   auto root = window.getCanvas();

   auto [scissorWidget, node] = make_node<ScissorWidget>("scissorWidget", "someText");
   root->addChild(std::move(node));

   scissorWidget->setScissorArea({25, 25, 25, 25});

   window.exec();
   return 0;
}