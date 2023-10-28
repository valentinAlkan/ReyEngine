#pragma once
#include "Property.h"
namespace Theme {
   enum class Outline {
      NONE, LINE, SHADOW
   };

   struct OutlineTypeProperty : public EnumProperty<Outline, 3>{
      OutlineTypeProperty(const std::string& instanceName,  Outline defaultvalue)
      : EnumProperty<Outline, 3>(instanceName, defaultvalue),
      PROPERTY_DECLARE(color, GFCSDraw::ColorRGBA(0,0,0,255)),
      PROPERTY_DECLARE(thickness, 1.0)
      {}
      const EnumPair<Outline, 3>& getDict() override {return dict;}
      static constexpr EnumPair<Outline, 3> dict = {
         ENUM_PAIR_DECLARE(Outline, NONE),
         ENUM_PAIR_DECLARE(Outline, LINE),
         ENUM_PAIR_DECLARE(Outline, SHADOW),
      };
      void registerProperties() override {
         registerProperty(color);
         registerProperty(thickness);
      }
      ColorProperty color;
      FloatProperty thickness;
   };
   
   enum class Background{
      NONE, SOLID, GRADIENT
   };

   struct BackgroundProperty : public EnumProperty<Background, 3>{
      BackgroundProperty(const std::string& instanceName,  Background defaultvalue)
      : EnumProperty<Background, 3>(instanceName, defaultvalue)
      {}
      const EnumPair<Background, 3>& getDict() override {return dict;}
      static constexpr EnumPair<Background, 3> dict = {
         ENUM_PAIR_DECLARE(Background, NONE),
         ENUM_PAIR_DECLARE(Background, SOLID),
         ENUM_PAIR_DECLARE(Background, GRADIENT),
      };
   };
}



