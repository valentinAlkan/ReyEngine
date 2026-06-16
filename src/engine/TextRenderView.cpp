#include "TextRenderView.h"

using namespace std;
using namespace ReyEngine;

/////////////////////////////////////////////////////////////////////////////////////////
void TextRenderView::_init(){
   if (_renderModel.empty()) _renderModel = getName();
   theme->background.fill = Style::Fill::NONE;
}

/////////////////////////////////////////////////////////////////////////////////////////
void TextRenderView::setText(const std::string& newText){
   _renderModel = newText;
}

/////////////////////////////////////////////////////////////////////////////////////////
void TextRenderView::render2D(RenderContext&) const {
   drawText(_renderModel, {0, 0}, theme->font);
}
