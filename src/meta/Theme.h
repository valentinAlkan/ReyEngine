#pragma once
#include "Property.h"
#include "Font.hpp"
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
      PROPERTY_DECLARE(color, ReyEngine::Colors::black),
      PROPERTY_DECLARE(thickness, 1.0)
      {}
      const EnumPair<Outline, 3>& getDict() const override {return dict;}
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
      using Property<Fill>::operator=;
      FillProperty(const std::string& instanceName,  Fill defaultvalue)
      : EnumProperty<Fill, 3>(instanceName, defaultvalue)
      , PROPERTY_DECLARE(colorPrimary, ReyEngine::Colors::none)
      , PROPERTY_DECLARE(colorSecondary, ReyEngine::Colors::none)
      , PROPERTY_DECLARE(colorTertiary, ReyEngine::Colors::none)
      {}
      const EnumPair<Fill, 3>& getDict() const override {return dict;}
      static constexpr EnumPair<Fill, 3> dict = {
         ENUM_PAIR_DECLARE(Fill, NONE),
         ENUM_PAIR_DECLARE(Fill, SOLID),
         ENUM_PAIR_DECLARE(Fill, GRADIENT),
      };
      void registerProperties() override {
         registerProperty(colorPrimary);
         registerProperty(colorSecondary);
         registerProperty(colorTertiary);
      }
      ColorProperty colorPrimary;
      ColorProperty colorSecondary; //for gradient
      ColorProperty colorTertiary; //for whatever
   };
   
   struct MarginsProperty : public Vec4Property<int>{
       using Property<ReyEngine::Vec4<int>>::operator=;
       MarginsProperty(const std::string& instanceName): Vec4Property<int>(instanceName, PropertyTypes::LayoutMargin){}
       int left(){return value.w;}
       int right(){return value.x;}
       int top(){return value.y;}
       int bottom(){return value.z;}
       void setLeft(int l){value.w = l;}
       void setRight(int r){value.x = r;}
       void setTop(int t){value.y = t;}
       void setBottom(int b){value.z = b;}
       void setAll(int a){value.w=a;value.x=a;value.y=a;value.z=a;}
   };

   struct CursorProperty : public Property<InputInterface::MouseCursor>{
      using Property<InputInterface::MouseCursor>::operator=;
      CursorProperty(const std::string& instanceName): Property<InputInterface::MouseCursor>(instanceName, PropertyTypes::Cursor, InputInterface::MouseCursor::DEFAULT){}
      std::string toString() const override {return std::to_string((int)value);}
      InputInterface::MouseCursor fromString(const std::string& data) override {return (InputInterface::MouseCursor)stoi(data);}
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
      PROPERTY_DECLARE(background, Fill::NONE),
      PROPERTY_DECLARE(foreground, Fill::SOLID),
      PROPERTY_DECLARE(roundness, 0.02),
      PROPERTY_DECLARE(roundnessSegments, 10),
      PROPERTY_DECLARE(font),
      PROPERTY_DECLARE(layoutMargins),
      PROPERTY_DECLARE(lineThick),
      PROPERTY_DECLARE(highlight, ReyEngine::Colors::blue),
      PROPERTY_DECLARE(cursor)
      {
         //apply defaults
         background = Fill::SOLID;
         background.colorPrimary = ReyEngine::Colors::lightGray;
         background.colorSecondary = ReyEngine::Colors::black;
         background.colorTertiary = ReyEngine::Colors::white;
      }
      std::string toString() const override {return "not impelenmeted";}
      Empty fromString(const std::string& data) override { return value;}

      FillProperty background;
      FillProperty foreground;
      ColorProperty highlight;
      OutlineProperty outline;
      FloatProperty roundness;
      IntProperty roundnessSegments;
      FontProperty font;
      MarginsProperty layoutMargins;
      FloatProperty lineThick;
      CursorProperty cursor;

      void registerProperties() override {
         registerProperty(background);
         registerProperty(foreground);
         registerProperty(outline);
         registerProperty(roundness);
         registerProperty(font);
         registerProperty(layoutMargins);
      }

      //Set the size of the font
      void setFontSize(float size){
         font.value.size = size;
      }

   private:
      Empty value; //do not use - only exists to satisfy construction requirements
   };
}
