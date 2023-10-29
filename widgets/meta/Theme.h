#pragma once
#include "Property.h"
namespace Style {
   /////////////////////////////////////////////////////////////////////////////////////////
   /////////////////////////////////////////////////////////////////////////////////////////
   /////////////////////////////////////////////////////////////////////////////////////////
   enum class Outline {
      NONE, LINE, SHADOW
   };

   /////////////////////////////////////////////////////////////////////////////////////////
   struct OutlineProperty : public EnumProperty<Outline, 3>{
      OutlineProperty(const std::string& instanceName,  Outline defaultvalue)
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

   /////////////////////////////////////////////////////////////////////////////////////////
   /////////////////////////////////////////////////////////////////////////////////////////
   /////////////////////////////////////////////////////////////////////////////////////////
   enum class Background{
      NONE, SOLID, GRADIENT
   };

   /////////////////////////////////////////////////////////////////////////////////////////
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

   /////////////////////////////////////////////////////////////////////////////////////////
   /////////////////////////////////////////////////////////////////////////////////////////
   /////////////////////////////////////////////////////////////////////////////////////////
   // A syntax to define themes
   class Stylesheet {

   };

   /////////////////////////////////////////////////////////////////////////////////////////
   /////////////////////////////////////////////////////////////////////////////////////////
   /////////////////////////////////////////////////////////////////////////////////////////
   //A theme, the underlying class that controls widgets' appearances
   struct Empty{}; //does nothing
   class Theme : public Property<Empty> {
   public:
      Theme(const std::string& instanceName = "DefaultTheme")
      : Property<Empty>(instanceName, PropertyTypes::Theme, Empty()),
      PROPERTY_DECLARE(outline, Outline::NONE),
      PROPERTY_DECLARE(background, Background::NONE)
      {}
      std::string toString() override {return "not impelenmeted";}
      Empty fromString(const std::string& data) override { return value;}

      BackgroundProperty background;
      OutlineProperty outline;
   private:
      Empty value; //do not use - only exists to satisfy construction requirements
   };

   class Themeable : public PropertyContainer {
   public:
      Themeable() = default;
      void registerProperties() override {
         registerProperty(theme);
      }
      Theme& getTheme(){return theme;}
      void setTheme(Theme){}
   private:
      Theme theme;
   };
}
