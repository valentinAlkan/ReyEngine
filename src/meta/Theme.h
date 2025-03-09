#pragma once
#include "ReyEngine.h"

namespace ReyEngine {
   namespace Style{
      /////////////////////////////////////////////////////////////////////////////////////////
      /////////////////////////////////////////////////////////////////////////////////////////
      /////////////////////////////////////////////////////////////////////////////////////////
      enum class Outline {
         NONE, LINE, SHADOW
      };

   //   /////////////////////////////////////////////////////////////////////////////////////////
   //   struct OutlineProperty : public EnumProperty<Outline, 3>{
   //      OutlineProperty(const std::string& instanceName,  Outline&& defaultvalue)
   //      : EnumProperty<Outline, 3>(instanceName, std::move(defaultvalue)),
   //      PROPERTY_DECLARE(color, ReyEngine::Colors::black),
   //      PROPERTY_DECLARE(thickness, 1.0)
   //      {}
   //      const EnumPair<Outline, 3>& getDict() const override {return dict;}
   //      static constexpr EnumPair<Outline, 3> dict = {
   //         ENUM_PAIR_DECLARE(Outline, NONE),
   //         ENUM_PAIR_DECLARE(Outline, LINE),
   //         ENUM_PAIR_DECLARE(Outline, SHADOW),
   //      };
   //      void registerProperties() override {
   //         registerProperty(color);
   //         registerProperty(thickness);
   //      }
   //      ReyEngine::ColorProperty color;
   //      FloatProperty thickness;
   //   };

      /////////////////////////////////////////////////////////////////////////////////////////
      /////////////////////////////////////////////////////////////////////////////////////////
      /////////////////////////////////////////////////////////////////////////////////////////
      enum class Fill {
         NONE, SOLID, GRADIENT
      };
      struct StyleRole {
         ColorRGBA colorPrimary; //for gradient
         ColorRGBA colorSecondary; //for gradient
         ColorRGBA colorTertiary; //for whatever
         ColorRGBA colorDisabled; //for whatever
         Fill fill = Fill::NONE;
         Outline outline = Outline::NONE;
         float roundness = 0.2;
         float linethick = 1;
      };

   //   enum class

   //   /////////////////////////////////////////////////////////////////////////////////////////
   //   struct FillProperty : public EnumProperty<Fill, 3>{
   //      using Property<Fill>::operator=;
   //      FillProperty(const std::string& instanceName,  Fill&& defaultvalue)
   //      : EnumProperty<Fill, 3>(instanceName, std::move(defaultvalue))
   //      , PROPERTY_DECLARE(colorPrimary, ReyEngine::Colors::none)
   //      , PROPERTY_DECLARE(colorSecondary, ReyEngine::Colors::none)
   //      , PROPERTY_DECLARE(colorTertiary, ReyEngine::Colors::none)
   //      , PROPERTY_DECLARE(colorDisabled, ReyEngine::Colors::lightGray)
   //      {}
   //      const EnumPair<Fill, 3>& getDict() const override {return dict;}
   //      static constexpr EnumPair<Fill, 3> dict = {
   //         ENUM_PAIR_DECLARE(Fill, NONE),
   //         ENUM_PAIR_DECLARE(Fill, SOLID),
   //         ENUM_PAIR_DECLARE(Fill, GRADIENT),
   //      };
   //      void registerProperties() override {
   //         registerProperty(colorPrimary);
   //         registerProperty(colorSecondary);
   //         registerProperty(colorTertiary);
   //         registerProperty(colorDisabled);
   //      }
   //      ReyEngine::ColorProperty colorPrimary;
   //      ReyEngine::ColorProperty colorSecondary; //for gradient
   //      ReyEngine::ColorProperty colorTertiary; //for whatever
   //      ReyEngine::ColorProperty colorDisabled; //for whatever
   //   };

      template <typename T>
      struct Margins {
          int left(){return value.w;}
          int right(){return value.x;}
          int top(){return value.y;}
          int bottom(){return value.z;}
          void setLeft(T l){value.w = l;}
          void setRight(T r){value.x = r;}
          void setTop(T t){value.y = t;}
          void setBottom(T b){value.z = b;}
          void setAll(T a){value.w=a;value.x=a;value.y=a;value.z=a;}
      private:
          ReyEngine::Vec4<T> value;
      };

   //   struct CursorProperty : public Property<InputInterface::MouseCursor>{
   //      using Property<InputInterface::MouseCursor>::operator=;
   //      CursorProperty(const std::string& instanceName): Property<InputInterface::MouseCursor>(instanceName, PropertyTypes::Cursor, InputInterface::MouseCursor::DEFAULT){}
   //      std::string toString() const override {return std::to_string((int)value);}
   //      InputInterface::MouseCursor fromString(const std::string& data) override {return (InputInterface::MouseCursor)stoi(data);}
   //   };

      /////////////////////////////////////////////////////////////////////////////////////////
      /////////////////////////////////////////////////////////////////////////////////////////
      /////////////////////////////////////////////////////////////////////////////////////////
      // A syntax to define themes
      class Stylesheet {

      };
   }
   /////////////////////////////////////////////////////////////////////////////////////////
   /////////////////////////////////////////////////////////////////////////////////////////
   /////////////////////////////////////////////////////////////////////////////////////////
   //A theme, the underlying class that controls widgets' appearances
   class Theme {
   public:
      Theme(const std::string& instanceName = "DefaultTheme"){
         //apply defaults
         background.fill = Style::Fill::SOLID;
         background.colorPrimary = ReyEngine::Colors::lightGray;
         background.colorSecondary = ReyEngine::Colors::black;
         background.colorTertiary = ReyEngine::Colors::white;
         background.colorDisabled = ReyEngine::Colors::disabledGray;
         foreground.colorDisabled = ReyEngine::Colors::white;
      }

      Style::StyleRole background;
      Style::StyleRole foreground;
      Style::StyleRole highlight;
      Style::StyleRole outline;
      ReyEngine::ReyEngineFont font;
      Style::Margins<float> layoutMargins;
      InputInterface::MouseCursor cursor;
      float segments = 5; //used for roundness

      //Set the size of the font
      void setFontSize(float size){
         font.size = size;
      }
   };
}