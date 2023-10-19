#pragma once

#include "Control.hpp"

class Slider : public Control {
public:
   enum class SliderType{VERTICAL, HORIZONTAL};
   struct SliderTypeProperty : public Property<SliderType>{
      SliderTypeProperty(const std::string& instanceName, SliderType defaultvalue = SliderType::VERTICAL)
            : Property(instanceName, PropertyTypes::Int, defaultvalue)
      {}
      std::string toString() override {return value == SliderType::VERTICAL ? "VERTICAL" : "HORIZONTAL";}
      SliderType fromString(const std::string& str) override {return (str == "VERTICAL" ? SliderType::VERTICAL : SliderType::HORIZONTAL);}
   };

   static std::shared_ptr<BaseWidget> deserialize(const std::string &instanceName, PropertyPrototypeMap &properties) {
      const GFCSDraw::Rect<float> &r = {0, 0, 0, 0};
      SliderTypeProperty temp("temp");
      auto retval = std::make_shared<Slider>(instanceName, r, temp.fromString(properties["sliderType"].data));
      retval->BaseWidget::_deserialize(properties);
      return retval;
   }
   Slider(const std::string &name, const GFCSDraw::Rect<float> &r, SliderType sliderType) : Slider(name, "Slider", r, sliderType) {}
protected:
   void _register_parent_properties() override{ Control::_register_parent_properties(); Control::registerProperties();}
   Slider(const std::string &name, const std::string &typeName, const GFCSDraw::Rect<float>& r, SliderType sliderDir)
   : Control(name, typeName, r)
   , sliderValue("sliderValue", 0.0)
   , minSliderValue("minSliderValue", 0.0)
   , maxSlidervalue("maxSliderValue", 100.0)
   , sliderType("sliderType", sliderDir)
   {
      grabber.width = _rect.value.width;
      grabber.height = _rect.value.height / 10;
   }
   virtual Handled _unhandled_input(InputEvent& e){
      bool handled = false;
      if (e.eventType == InputEventMouseMotion::EVENT_INPUT_MOUSE_MOTION) {
         auto& mouseEvent = (InputEventMouseMotion&)(e);
         auto localPos = globalToLocal(mouseEvent.globalPos);
         _cursor_in_slider = isInside(localPos);
         _cursor_in_grabber = grabber.isInside(localPos);

         //drag grabber
         if (_is_dragging){
            grabber.setCenter(localPos);
         }
         return true;
      }

      if (e.eventType == InputEventMouseButton::EVENT_INPUT_MOUSE_BUTTON) {
         auto& mouseEvent = (InputEventMouseButton&)(e);
         auto localPos = globalToLocal(mouseEvent.globalPos);
         _cursor_down = mouseEvent.isDown;
         _is_dragging = _cursor_in_grabber && _cursor_down;
         return true;
      }
   }
   void render() const override {
      //draw slider
      _drawRectangle(_rect.value.toSizeRect(), _cursor_in_slider ? GREEN : RED);
      //draw grabber
      _drawRectangle(grabber, _cursor_down && _cursor_in_grabber ? YELLOW : BLUE);
   }
   void _process(float dt) override {}
   void registerProperties() override{}

   FloatProperty sliderValue;
   FloatProperty minSliderValue;
   FloatProperty maxSlidervalue;
   SliderTypeProperty sliderType;
   bool _cursor_in_slider = false;
   bool _cursor_in_grabber = false;
   bool _cursor_down = false;
   bool _is_dragging = false;
   GFCSDraw::Rect<float> grabber = {0,0,0,0};
};