#include "Window.h"
#include "Label.h"
#include "Table.h"

using namespace ReyEngine;
using namespace std;

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
   Handled __process_unhandled_input(const InputEvent& event) override {
      Logger::info() << getName() << " processing input " << event.isMouse().value()->getLocalPos() << endl;
      if (event.isMouse().value()->isInside()) Logger::info() << "Which is inside" << endl;
      mousePos = event.isMouse().value()->getLocalPos();
      highlight = event.isMouse().value()->isInside();
      if (isFocused()) {
         Logger::info() << "Handling focused" << endl;
         return this;
      }
      return nullptr;
   }
   const ColorRGBA color;
   bool highlight = false;
   Pos<float> mousePos;
};

int main() {
   auto& window = Application::createWindowPrototype("window", 800, 800, {WindowFlags::RESIZE}, 60)->createWindow();
   auto root = window.getCanvas();

   // auto testCanvas1 = make_child<TestCanvas>(root, "testCanvas1", Colors::blue);
   // testCanvas1->setRect({50,50,500,500});
   // testCanvas1->setFocused(true);
   //
   // auto testCanvas2 = make_child<TestCanvas>(testCanvas1, "testCanvas2", Colors::yellow);
   // testCanvas2->setRect({50,50,400,400});
   //
   // auto testCanvas3 = make_child<TestCanvas>(testCanvas2, "testCanvas3", Colors::green);
   // testCanvas3->setRect({50,50,300,300});
   //
   // make_child<Grid>(root, "grid")->setAnchoring(Anchor::FILL);

   auto scrollArea = make_child<ScrollArea>(root, "scroll area");
   scrollArea->setRect(100,100,600,600);

   auto subgrid = make_child<Grid>(scrollArea, "subgrid");
   subgrid->setAnchoring(Anchor::FILL);

   auto label = make_child<Label>(scrollArea, "label");
   label->setPosition(1500,1500);

   InputEventMouseMotion motion(&window, {202,202}, {0,0});
   root->processInput(motion);

   window.exec();
   return 0;
}