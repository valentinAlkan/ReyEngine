#pragma once
#include "Control.hpp"

// A simple visual panel that just gives us something to look at. No other functionality.
class Panel : public Control {
public:
   static constexpr std::string_view LAYOUT_NAME = "layout";
   GFCSDRAW_OBJECT(Panel, Control)
   , PROPERTY_DECLARE(layoutDir, Layout::LayoutDir::HORIZONTAL)
   {
      getTheme()->background.colorPrimary.set(GFCSDraw::ColorRGBA(156, 181, 156, 255));
   }
public:
   void render() const override {
      auto roundness = getThemeReadOnly().roundness.get();
      auto color = getThemeReadOnly().background.colorPrimary.get();
      _drawRectangleRounded(_rect.value.toSizeRect(), roundness, 1, color);
   }
   void _init() override {}
   void _process(float dt) override {}
   void setLayout(){
      removeChild(std::string(LAYOUT_NAME));
   };
   std::optional<std::shared_ptr<Layout>> getLayout(){
      auto child = getChild("layout");
      if (child){
         return std::dynamic_pointer_cast<Layout>(child.value());
      }
      return std::nullopt;
   };
   void registerProperties() override{
      //register properties specific to your type here.
      registerProperty(layoutDir);
   }
   Layout::LayoutProperty layoutDir;
};