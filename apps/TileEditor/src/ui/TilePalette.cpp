#include "TilePalette.h"

using namespace std;
using namespace ReyEngine;

/////////////////////////////////////////////////////////////////////////////////////////
TilePalette::TilePalette()
: Layout(LayoutDir::GRID)
{

}

/////////////////////////////////////////////////////////////////////////////////////////
void TilePalette::_init() {
   Layout::_init();
   setSize(THUMBNAIL_SIZE.x * 5, THUMBNAIL_SIZE.y * 15);
}

/////////////////////////////////////////////////////////////////////////////////////////
void TilePalette::render2D() const {
   //draw a tile palette grid
   drawRectangle(getSizeRect(), Colors::blue);
   for (int x=0; (float)x<=getWidth(); /**/){
      for (int y=0; (float)y<=getHeight(); /**/){
         Pos<float> a = {0, (float)y};
         Pos<float> b = a + Pos<float>(getWidth(), 0);
         drawLine({a,b}, 1.0, Colors::black);
         y += THUMBNAIL_SIZE.y;
      }
      Pos<float> a = {(float)x, 0};
      Pos<float> b = a + Pos<float>(0, getHeight());
      drawLine({a,b}, 1.0, Colors::black);
      x += THUMBNAIL_SIZE.x;
   }


}

/////////////////////////////////////////////////////////////////////////////////////////
Widget *TilePalette::_unhandled_input(const ReyEngine::InputEvent& event) {
   if (event.isMouse() && event.isMouse().value()->isInside()) {
//      getSizeRect().getSubRectIndex()
      return this;
   }
   return nullptr;
}