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
      PROPERTY_DECLARE(color, GFCSDraw::Colors::black),
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
   enum class Fill {
      NONE, SOLID, GRADIENT
   };

//   enum class

   /////////////////////////////////////////////////////////////////////////////////////////
   struct FillProperty : public EnumProperty<Fill, 3>{
      FillProperty(const std::string& instanceName,  Fill defaultvalue)
      : EnumProperty<Fill, 3>(instanceName, defaultvalue)
      , PROPERTY_DECLARE(colorPrimary, GFCSDraw::Colors::none)
      , PROPERTY_DECLARE(colorSecondary, GFCSDraw::Colors::none)
      {}
      const EnumPair<Fill, 3>& getDict() override {return dict;}
      static constexpr EnumPair<Fill, 3> dict = {
         ENUM_PAIR_DECLARE(Fill, NONE),
         ENUM_PAIR_DECLARE(Fill, SOLID),
         ENUM_PAIR_DECLARE(Fill, GRADIENT),
      };
      void registerProperties() override {
         registerProperty(colorPrimary);
         registerProperty(colorSecondary);
      }
      ColorProperty colorPrimary;
      ColorProperty colorSecondary; //for gradient
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
      PROPERTY_DECLARE(background, Fill::NONE)
      {}
      std::string toString() override {return "not impelenmeted";}
      Empty fromString(const std::string& data) override { return value;}

      FillProperty background;
      OutlineProperty outline;
   private:
      Empty value; //do not use - only exists to satisfy construction requirements
   };

   class Themeable : public PropertyContainer {
   public:
      Themeable(){
         theme = std::make_unique<Theme>();
      };
      void registerProperties() override {
         registerProperty(*theme);
      }
      const Theme& refTheme() const {return *theme;}
      std::unique_ptr<Theme>& getTheme() {return theme;}
      void setTheme(std::unique_ptr<Theme> newTheme){
         theme = std::move(newTheme);
         updateProperty(*theme);
      }
   private:
      std::unique_ptr<Theme> theme;
   };
}
