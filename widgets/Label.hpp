#pragma once
#include "Control.hpp"
#include "StringTools.h"
#include "Theme.h"

class Label : public Control {
   GFCSDRAW_OBJECT(Label, Control)
   , PROPERTY_DECLARE(text, getName())
   , PROPERTY_DECLARE(outline, Theme::Outline::NONE)
   {}
public:
   void render() const override{
      //todo: scissor text
      switch(outline.value){
         case Theme::Outline::LINE:
            _drawRectangleLines(getRect(), outline.thickness.get(), outline.color.get());
            break;
         case Theme::Outline::SHADOW:
            break;
         default:
            break;
      }
       _drawText(text.value, {0, 0}, 20, GFCSDraw::Colors::black);
   };
   void _process(float dt) override {};
   Theme::Outline getOutlineType(Theme::Outline outlineType){ return outline.get();}
   void setOutlineType(Theme::Outline outlineType){outline.set(outlineType);}
   void registerProperties() override{
      registerProperty(text);
   };
   void setText(const std::string& newText){
      text.set(newText);
//      auto textSize = GFCSDraw::measureText(GFCSDraw::getDefaultFont(), text.get().c_str(), 20, 1);
      auto textWidth = MeasureText(newText.c_str(), 20);
      setRect({0,0,(double)textWidth, 20});
   }
protected:
   StringProperty text;
   Theme::OutlineTypeProperty outline;
};
