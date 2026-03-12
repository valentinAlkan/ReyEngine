#include "Window.h"
#include "Label.h"
#include "Table.h"

using namespace ReyEngine;
using namespace std;

Size<float> windowSize = {800,600};


class Grid : public Widget {
public:
   REYENGINE_OBJECT(Grid)
   void render2D() const override {
      for (int x=0; x<windowSize.x; x+=50) {
         for (int y=0; y<windowSize.y; y+=50) {
            drawLine(Line<float>({0,(float)y},{windowSize.x,(float)y}), 1.0, Colors::black);
         }
         drawLine(Line<float>({(float)x, 0},{(float)x, windowSize.y}), 1.0, Colors::black);
      }
   }
};

class TestCanvas : public Canvas {
   public:
   REYENGINE_OBJECT(TestCanvas)
   TestCanvas()
   :color(ColorRGBA::random(255))
   {
   }
   void render2D() const override {
      drawRectangle(getSizeRect(), color);
   }
   void _init() override {
      make_child<Label>(this, "TestCanvasLabel", getName());
   }
   const ColorRGBA color;
};

int main() {
   auto& window = Application::createWindowPrototype("window", windowSize.x, windowSize.y, {WindowFlags::RESIZE}, 60)->createWindow();
   auto root = window.getCanvas();

   auto testCanvas1 = make_child<TestCanvas>(root, "testCanvas1");
   testCanvas1->setRect({50,50,500,500});

   auto testCanvas2 = make_child<TestCanvas>(testCanvas1, "testCanvas2");
   testCanvas2->setRect({50,50,400,400});

   auto testCanvas3 = make_child<TestCanvas>(testCanvas2, "testCanvas3");
   testCanvas3->setRect({50,50,300,300});

   auto grid = make_child<Grid>(root, "grid");

   window.exec();
   return 0;
}