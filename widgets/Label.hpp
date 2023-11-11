#pragma once
#include "Control.hpp"
#include "StringTools.h"
#include "Theme.h"
#include <iomanip>
#include <sstream>

class Label : public Control {
   GFCSDRAW_OBJECT(Label, Control)
   , PROPERTY_DECLARE(text, getName())
   {}
public:
   void render() const override{
      //todo: scissor text
      auto& outline = getThemeReadOnly().outline;
      auto& background = getThemeReadOnly().background;
      auto& foreground = getThemeReadOnly().foreground;
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
       _drawText(text.value, {0, 0}, getThemeReadOnly().font.value);
   };
   void _process(float dt) override {};
   void registerProperties() override{
      registerProperty(text);
   };
   void setText(const std::string& newText){
      text.set(newText);
      if (!isInLayout) {
         auto textWidth = MeasureText(newText.c_str(), 20);
         auto pos = getPos();
         setRect({pos.x, pos.y, textWidth, 20});
      }
   }
   //precision refers to how many decimal places should appear
   void setText(double newText, int precision){
      auto textRepr = std::to_string(newText);
      auto digitsCount = string_tools::decimalCount(textRepr);
      auto integerCount = digitsCount.value().first;
      auto decimalCount = digitsCount.value().second;
      setText(std::string(textRepr, 0, integerCount + 1 + (decimalCount > precision ? precision : decimalCount)));
   }
   void setText(int newText){
      setText(std::to_string(newText));
   }
   void calculateTextWidth(){
      //calculate the width of the text in the label
//      GFCSDraw::measureText(GetFontDefault(), text.value, )
   }
protected:
   StringProperty text;
};
