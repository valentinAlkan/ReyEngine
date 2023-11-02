#pragma once
#include "Control.hpp"
#include "StringTools.h"
#include "Theme.h"

class Label : public Control {
   GFCSDRAW_OBJECT(Label, Control)
   , PROPERTY_DECLARE(text, getName())
   {}
public:
   void render() const override{
      //todo: scissor text
      auto& outline = refTheme().outline;
      auto& background = refTheme().background;
      auto& foreground = refTheme().foreground;
      switch (background.value){
         case Style::Fill::SOLID:
            _drawRectangle(_rect.value.toSizeRect(), background.colorPrimary.value);
         default:
            break;
      }
      switch(outline.value){
         case Style::Outline::LINE:
            _drawRectangleLines(getRect(), outline.thickness.get(), outline.color.get());
            break;
//         case Style::Outline::SHADOW:
//            break;
         default:
            break;
      }
       _drawText(text.value, {0, 0}, 20, foreground.value);
   };
   void _process(float dt) override {};
   void registerProperties() override{
      registerProperty(text);
   };
   void setText(const std::string& newText){
      text.set(newText);
//      auto textSize = GFCSDraw::measureText(GFCSDraw::getDefaultFont(), text.get().c_str(), 20, 1);
      if (!isInLayout) {
         auto textWidth = MeasureText(newText.c_str(), 20);
         auto pos = getPos();
         setRect({pos.x, pos.y, textWidth, 20});
      }
   }
protected:
   StringProperty text;
};
