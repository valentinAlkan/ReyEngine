#include "Window.h"
#include "Label.h"
#include "Table.h"
#include "Button.h"
#include <cassert>
#include "FileBrowser.h"

using namespace ReyEngine;
using namespace std;

class PointDrawer : public Widget {
public:
   REYENGINE_OBJECT(PointDrawer)
   PointDrawer() = default;
   void render2D() const override {
      auto pt = InputManager::getMousePos().get();
      drawText(pt.toString(), pt + Pos<float>(10,10), theme->font);
   }


};

class Grid : public Widget {
public:
   REYENGINE_OBJECT(Grid)
   void render2D() const override {
      auto size = getSize();
      for (int x=0; x<size.x; x+=50) {
         for (int y=0; y<size.y; y+=50) {
            drawLine(Line<float>({0,(float)y},{size.x,(float)y}), 1.0, Colors::black);
         }
         drawLine(Line<float>({(float)x, 0},{(float)x, size.y}), 1.0, Colors::black);
      }
   }
};

class TestCanvas : public Canvas {
public:
   REYENGINE_OBJECT(TestCanvas)
   TestCanvas(const ColorRGBA& color)
   :color(color)
   {
   }
   void render2D() const override {
      drawRectangle(getSizeRect(), color);
      if (highlight) {
         drawRectangleLines(getSizeRect().embiggen(-1), 2.0, Colors::red);
      }
      drawText(mousePos, mousePos + Pos<float>(20,20), theme->font);
   }
   void _init() override {
      setToolTipText(getName());
      auto label = make_child<Label>(this, "TestCanvasLabel", getName());
      label->setToolTipText(label->getText() + "label size = " + label->getSize().toString());
   }
   Handled _unhandled_input(const InputEvent& event) override {
      Logger::info() << getName() << " processing input " << event.isMouse().value()->getLocalPos() << endl;
      if (event.isMouse().value()->isInside()) Logger::info() << "Which is inside" << endl;
      mousePos = event.isMouse().value()->getLocalPos();
      highlight = event.isMouse().value()->isInside();
      if (const auto isMouse = event.isMouse(); isMouse.value()->isInside()) {
         Logger::info() << "Handling input @ " << mousePos << endl;
         return {this, mousePos};
      }
      return {};
   }
   const ColorRGBA color;
   bool highlight = false;
   Pos<float> mousePos;
};

int main() {
   auto& window = Application::createWindowPrototype("window", 1280, 1024, {WindowFlags::RESIZE}, 60)->createWindow();
   auto root = window.getCanvas();

   std::shared_ptr<Canvas> testCanvas1;
   std::shared_ptr<Canvas> testCanvas2;
   std::shared_ptr<Canvas> testCanvas3;

   testCanvas1 = make_child<TestCanvas>(root, "testCanvas1", Colors::blue);
   testCanvas1->setRect({50,50,500,500});
   {
      InputEventMouseMotion motion(&window, {51,51}, {0,0});
      auto handler = root->processInput(motion);
      if (handler) {
         Logger::info() << "Input handled by " << handler.handler->getName() << " @ " << handler.pos.value() << endl;
      } else {
         Logger::error() << "Input unhandled!" << endl;
      }
      assert(handler.handler == testCanvas1.get());
      assert(handler.pos == Pos<float>(1, 1));
   }

   {
      InputEventMouseMotion motion(&window, {124,113}, {0,0});
      testCanvas2 = make_child<TestCanvas>(testCanvas1, "testCanvas2", Colors::yellow);
      testCanvas2->setRect({50,50,400,400});
      auto handler = root->processInput(motion);
      assert(handler.handler == testCanvas2.get());
      assert(handler.pos == Pos<float>(24,13));
   }
   {
      InputEventMouseMotion motion(&window, {347,399}, {0,0});
      testCanvas3 = make_child<TestCanvas>(testCanvas2, "testCanvas3", Colors::green);
      testCanvas3->setRect({50,50,300,300});

      auto handler = root->processInput(motion);
      assert(handler.handler == testCanvas3.get());
      assert(handler.pos == Pos<float>(197, 249));
   }

   root->removeAllChildren();

   auto scrollArea = make_child<ScrollArea>(root, "scroll area");
   scrollArea->setRect(100,100,600,600);

   auto label = make_child<Label>(scrollArea, "GridLabel!");
   label->setPosition(1000,1000);

   auto subgrid = make_child<Grid>(scrollArea, "subgrid");
   subgrid->setAnchoring(Anchor::FILL);

   auto pushButton = make_child<PushButton>(subgrid, "pushButton");
   pushButton->setPosition(200,200);

   //so far so good, lets test offset stuff
   scrollArea->setOffsetX(100);
   scrollArea->setOffsetY(100);

   //interact with the pushbutton
   {
      InputEventMouseHover hover(&window, {201,201});
      auto handler = root->processInput(hover);
      assert(handler.handler == pushButton.get());
      assert(handler.pos.value().x < 6);
      assert(handler.pos.value().y < 6);
   }

   auto filebrowser = make_child<FileBrowser>(root, "filebrowser");
   filebrowser->centerOnPoint(root->getCenter());
   filebrowser->open();
   assert(filebrowser->getVisible());

   //interact with the filebrowser
   {
      static constexpr Pos<float> pt = {948, 764};
      InputEventMouseButton down(&window, pt, InputInterface::MouseButton::LEFT, true, false);
      InputEventMouseButton up(&window, pt, InputInterface::MouseButton::LEFT, false, false);
      root->processInput(down);
      root->processInput(up);
      assert(!filebrowser->getVisible());
   }

   root->removeAllChildren();

   make_child<PointDrawer>(root, "pointDrawer")->moveToForeground();
   window.exec();
   return 0;
}