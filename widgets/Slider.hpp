#pragma once

#include "Control.hpp"

class Slider : public Control {
public:
   class SliderValueChangedEvent : public Event<BaseEvent> {
   public:
      EVENT_CTOR_SIMPLE(SliderValueChangedEvent, Event<BaseEvent>){}
      double value;
   };
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
   GFCSDraw::Vec2<double> getRange(){return _range;}
   void setRange(GFCSDraw::Vec2<double> newRange){
      minSliderValue.set(newRange.x);
      maxSlidervalue.set(newRange.y);
      _range = {minSliderValue.value, maxSlidervalue.value};
   }
   inline double getSliderValue(){return sliderValue.get();}
   inline double setSliderValue(double value){sliderValue.set(value);}
   inline double getSliderPct(){return _range.pct(sliderValue.value);}
   inline void setSliderPct(double value){sliderValue.set(_range.pct(sliderValue.value)/100.0);}
protected:
   void _register_parent_properties() override{ Control::_register_parent_properties(); Control::registerProperties();}
   Slider(const std::string &name, const std::string &typeName, const GFCSDraw::Rect<float>& r, SliderType sliderDir)
   : Control(name, typeName, r)
   , sliderValue("sliderValue", 0.0)
   , minSliderValue("minSliderValue", 0.0)
   , maxSlidervalue("maxSliderValue", 100.0)
   , sliderType("sliderType", sliderDir)
   {
      _grabber.width = _rect.value.width;
      _grabber.height = _rect.value.height / 10;
      _range = {minSliderValue.get(), maxSlidervalue.get()};
   }
   virtual Handled _unhandled_input(InputEvent& e){
      if (e.isEvent<InputEventMouseMotion>()) {
         auto& mouseEvent = (InputEventMouseMotion&)(e);
         auto localPos = globalToLocal(mouseEvent.globalPos);
         _cursor_in_slider = isInside(localPos);
         _cursor_in_grabber = _grabber.isInside(localPos);

         //drag grabber
         if (_is_dragging){
            //set new slider value based on input
            double newValue = 0;
            switch(sliderType.get()){
               case SliderType::VERTICAL: {
                  newValue = _range.lerp(_range.pct(localPos.y));
                  GFCSDraw::Vec2<double> adjustedRange = {0, _rect.get().height-_grabber.height};
                  sliderValue.set(_range.clamp(newValue));
                  _grabber.y = adjustedRange.lerp(_range.pct(sliderValue.get()));
               }
               break;
               case SliderType::HORIZONTAL:
                  newValue = _range.lerp(_range.pct(localPos.x));
                  GFCSDraw::Vec2<double> adjustedRange = {0, _rect.get().width-_grabber.width};
                  sliderValue.set(_range.clamp(newValue));
                  _grabber.y = adjustedRange.lerp(_range.pct(sliderValue.get()));
            }
            auto event = SliderValueChangedEvent(EventPublisher::shared_from_this());
            event.value = sliderValue.get();
            publish<SliderValueChangedEvent>(event);
         }
         return true;
      }

      if (e.isEvent<InputEventMouseButton>()){
         auto& mouseEvent = (InputEventMouseButton&)(e);
         _cursor_down = mouseEvent.isDown;
         _is_dragging = _cursor_in_grabber && _cursor_down;
         return true;
      }
   }
   void render() const override {
      //draw slider
      _drawRectangle(_rect.value.toSizeRect(), _cursor_in_slider ? GREEN : RED);
      //draw grabber
      _drawRectangle(_grabber, _cursor_down && _cursor_in_grabber ? YELLOW : BLUE);
   }
   void _on_rect_changed() override {

//      switch(SliderType::VERTICAL){
//            case SliderType::VERTICAL:
//            break;
//            case SliderType::HORIZONTAL:
//            break;
//         }
   };
   void _process(float dt) override {}
   void registerProperties() override{
      registerProperty(sliderValue);
      registerProperty(minSliderValue);
      registerProperty(maxSlidervalue);
      registerProperty(sliderType);
   }
private:
   void _compute_grabber(){
      //find the midpoint of the short edge of the slider
      auto grabberCenter = _range.lerp(sliderValue.get());
      GFCSDraw::Vec2<double> longSide;
      GFCSDraw::Vec2<double> shortSide;
      GFCSDraw::Line<double> travelLine; //the line the grabber travels along
      switch(SliderType::VERTICAL){
         case SliderType::VERTICAL:
            longSide = {0, _rect.get().height};
            shortSide = {0, _rect.get().width};
            travelLine = {shortSide.midpoint(), longSide.midpoint()};
            // lerp the longside
         break;
         case SliderType::HORIZONTAL:
            longSide = {0, _rect.get().width};
            shortSide = {0, _rect.get().height};
         break;
      }
      _grabber.setCenter(travelLine.lerp(_range.lerp(sliderValue.get())));
   }

   FloatProperty sliderValue;
   FloatProperty minSliderValue;
   FloatProperty maxSlidervalue;
   SliderTypeProperty sliderType;
   bool _cursor_in_slider = false;
   bool _cursor_in_grabber = false;
   bool _cursor_down = false;
   bool _is_dragging = false;
   GFCSDraw::Rect<double> _grabber = {0, 0, 0, 0};
   GFCSDraw::Vec2<double> _range = {0,0};
};