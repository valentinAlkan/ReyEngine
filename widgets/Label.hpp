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
       _drawText(text.value, {0, 0}, 20, foreground.value);
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
      std::stringstream ss;
      ss << std::setprecision(precision) << newText;
      auto textrep = ss.str();
      auto existDecimal = string_tools::rcountUntil(textrep, '.');
      if (existDecimal == std::string::npos){
         //no decimal
         textrep += '.';
         textrep.append(precision, '0');
      } else if (existDecimal < precision){
         //has too few decimals
         auto fillNeeded = precision - existDecimal;
         textrep.append(fillNeeded, '0');
      }

      setText(textrep);
   }
   void setText(int newText){
      setText(std::to_string(newText));
   }
protected:
   StringProperty text;
};
