#pragma once
#include "Control.hpp"
#include "StringTools.h"

enum OutlineType {
   NONE, LINE, SHADOW
};

struct OutlineTypeProperty : public EnumProperty<OutlineType, 3>{
   OutlineTypeProperty(const std::string& instanceName,  OutlineType defaultvalue)
   : EnumProperty<OutlineType, 3>(instanceName, defaultvalue)
   {}
   const EnumPair<OutlineType, 3>& getDict() override {return dict;}
   static constexpr EnumPair<OutlineType, 3> dict = {
         ENUM_PAIR_DECLARE(OutlineType, NONE),
         ENUM_PAIR_DECLARE(OutlineType, LINE),
         ENUM_PAIR_DECLARE(OutlineType, SHADOW),
   };
};

class Label : public Control {
   GFCSDRAW_OBJECT(Label, Control)
   , PROPERTY_DECLARE(text)
   , PROPERTY_DECLARE(outlineType, OutlineType::NONE)
         {
      setText("Label");
   }
public:
   void render() const override{
      //todo: scissor text
      if (outlineType.value == LINE){

      }
      _drawText(text.value, {0,0}, 20, BLACK);
   };
   void _process(float dt) override {};
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
   OutlineTypeProperty outlineType;

};
