#include "TextRenderView.h"

using namespace std;
using namespace ReyEngine;

/////////////////////////////////////////////////////////////////////////////////////////
void TextRenderView::_init(){}

/////////////////////////////////////////////////////////////////////////////////////////
void TextRenderView::render2D(RenderContext&) const {
   drawRectangle(getSizeRect(), getTheme().background.colorPrimary);
   drawText(getText(), {0, 0}, theme->font);
}
