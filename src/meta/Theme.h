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
         ColorRGBA colorActive;
         ColorRGBA colorHighlight;
         Fill fill = Fill::NONE;
         Outline outline = Outline::NONE;
         float roundness = 0.2;
         float linethick = 1;
      };

      template <typename T>
      struct Margins {
         Margins() = default;
         Margins(const auto& iterable){*this = iterable;}
         Margins(const std::initializer_list<T>& init) {
             auto it = init.begin();
             if (init.size() >= 4) {
                value.w = *it++;  // left
                value.x = *it++;  // right
                value.y = *it++;  // top
                value.z = *it++;  // bottom
             }
         }
         Margins& operator=(std::initializer_list<T> init) {
            auto it = init.begin();
            if (init.size() >= 4) {
               value.w = *it++;
               value.x = *it++;
               value.y = *it++;
               value.z = *it++;
            }
            return *this;
         }
         int left(){return value.w;}
         int right(){return value.x;}
         int top(){return value.y;}
         int bottom(){return value.z;}
         void setLeft(T l){value.w = l;}
         void setRight(T r){value.x = r;}
         void setTop(T t){value.y = t;}
         void setBottom(T b){value.z = b;}
         void setAll(T a){value.w=a;value.x=a;value.y=a;value.z=a;}
         void operator=(const auto& iterable){value = iterable;}
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
         background.colorPrimary = Colors::lightGray;
         background.colorSecondary = Colors::black;
         background.colorTertiary = Colors::white;
         background.colorDisabled = Colors::disabledGray;
         background.colorActive = Colors::activeBlue;
         background.colorHighlight = Colors::blue;
         foreground.colorPrimary = Colors::black;
         foreground.colorDisabled = Colors::white;
         font = std::make_shared<ReyEngineFont>();
      }

      Style::StyleRole background;
      Style::StyleRole foreground;
      Style::StyleRole highlight;
      Style::StyleRole outline;
      std::shared_ptr<ReyEngine::ReyEngineFont> font;
      Style::Margins<float> layoutMargins; //only used for layouts - adds space around AND BETWEEN all children
      Style::Margins<float> widgetPadding; //used for all widget anchoring - adds pixel buffer around our edges - this is usually the one you want
      InputInterface::MouseCursor cursor;
      float segments = 5; //used for roundness

      //Set the size of the font
      void setFontSize(float size){
         font->size = size;
      }
   };
}