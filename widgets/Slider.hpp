#pragma once

#include "Control.hpp"

class Slider : public Control {
public:
   class SliderValueChangedEvent : public Event<BaseEvent> {
   public:
      EVENT_CTOR_SIMPLE(SliderValueChangedEvent, Event<BaseEvent>){}
      double value;
      double pct;
   };

   enum class SliderType{VERTICAL, HORIZONTAL};
   struct SliderTypeProperty : public Property<SliderType>{
      SliderTypeProperty(const std::string& instanceName, SliderType defaultvalue = SliderType::VERTICAL)
            : Property(instanceName, PropertyTypes::Float, defaultvalue)
      {}
      std::string toString() override {return value == SliderType::VERTICAL ? "VERTICAL" : "HORIZONTAL";}
      SliderType fromString(const std::string& str) override {return (str == "VERTICAL" ? SliderType::VERTICAL : SliderType::HORIZONTAL);}
   };

   GFCSDRAW_DECLARE_STATIC_CONSTEXPR_TYPENAME(Slider)
   Slider(const std::string &name, const GFCSDraw::Rect<float>& r, SliderType sliderDir)
         : Control(name, _get_static_constexpr_typename(), r)
         , sliderValue("sliderValue", 0.0)
         , minSliderValue("minSliderValue", 0.0)
         , maxSlidervalue("maxSliderValue", 100.0)
         , sliderType("sliderType", sliderDir)
   {
      switch(sliderType.get()) {
         case SliderType::VERTICAL:
            _grabber.width = _rect.value.width;
            _grabber.height = _rect.value.height / 10;
            break;
         case SliderType::HORIZONTAL:
            _grabber.width = _rect.value.width/10;
            _grabber.height = _rect.value.height;
            break;
      }
      _range = {minSliderValue.get(), maxSlidervalue.get()};
   }

   static std::shared_ptr<BaseWidget> deserialize(const std::string &instanceName, PropertyPrototypeMap &properties) {
      const GFCSDraw::Rect<float> &r = {0, 0, 0, 0};
      SliderTypeProperty temp("temp");
      auto retval = std::make_shared<Slider>(instanceName, r, temp.fromString(properties["sliderType"].data));
      retval->BaseWidget::_deserialize(properties);
      return retval;
   }
   GFCSDraw::Vec2<double> getRange(){return _range;}
   void setRange(GFCSDraw::Vec2<double> newRange){
      minSliderValue.set(newRange.x);
      maxSlidervalue.set(newRange.y);
      _range = {minSliderValue.value, maxSlidervalue.value};
   }
   inline double getSliderValue(){return sliderValue.get();}
   inline void setSliderValue(double value){sliderValue.set(value);_publish_slider_val();_compute_appearance();}
   inline double getSliderPct(){return _range.pct(sliderValue.value);}
   inline void setSliderPct(double pct){setSliderValue(_range.lerp(pct));}
protected:
   void _register_parent_properties() override{ Control::_register_parent_properties(); Control::registerProperties();}
   virtual Handled _unhandled_input(InputEvent& e){
      auto& mouseEvent = (InputEventMouseMotion&)(e);
      auto localPos = globalToLocal(mouseEvent.globalPos);
      if (mouseEvent.isEvent<InputEventMouseMotion>()){
         _cursor_in_slider = isInside(localPos);
         if (_is_dragging) {
            //drag grabber
            if (_is_dragging) {
               //set new slider value based on input
               double newValue = 0;
               switch (sliderType.get()) {
                  case SliderType::VERTICAL: {
                     auto heightRange = GFCSDraw::Vec2<double>(0, getRect().height);
                     newValue = _range.lerp(heightRange.pct(localPos.y));
                     sliderValue.set(_range.clamp(newValue));
                  }
                     break;
                  case SliderType::HORIZONTAL:
                     auto widthRange = GFCSDraw::Vec2<double>(0, getRect().width);
                     newValue = _range.lerp(widthRange.pct(localPos.x));
                     sliderValue.set(_range.clamp(newValue));
               }
               _compute_appearance();
               _publish_slider_val();
            }
            return true;
         }
      }

      if (e.isEvent<InputEventMouseButton>()){
         auto &buttonEvent = e.toEventType<InputEventMouseButton>();
         bool handled = false;
         if (isInside(localPos) && buttonEvent.isDown) {
            _cursor_down = true;
            _is_dragging = _grabber.isInside(localPos) && _cursor_down;
            return true;
         }
         if (_is_dragging && !buttonEvent.isDown) {
            //only handle button up if we are dragging
            _cursor_down = buttonEvent.isDown;
            _is_dragging = false;
            return true;
         }
      }
   }
   void render() const override {
      //draw slider
      _drawRectangle(_rect.value.toSizeRect(), _cursor_in_slider || _is_dragging? GREEN : RED);
      //draw grabber
      _drawRectangle(_grabber, _cursor_down && _cursor_in_grabber || _is_dragging ? YELLOW : BLUE);
   }
   void _on_rect_changed() override {
   };
   void _process(float dt) override {}
   void registerProperties() override{
      registerProperty(sliderValue);
      registerProperty(minSliderValue);
      registerProperty(maxSlidervalue);
      registerProperty(sliderType);
   }
private:
   void _publish_slider_val(){
      auto event = SliderValueChangedEvent(EventPublisher::shared_from_this());
      event.value = getSliderValue();
      event.pct = getSliderPct();
      publish<SliderValueChangedEvent>(event);
   }
   void _compute_appearance(){
      //find the midpoint of the short edge of the slider
      GFCSDraw::Vec2<double> longSide;
      GFCSDraw::Vec2<double> shortSide;
      GFCSDraw::Line<double> travelLine; //the line the grabber travels along
      switch(sliderType.get()){
         case SliderType::VERTICAL: {
            GFCSDraw::Vec2<double> adjustedRange = {0, _rect.get().height - _grabber.height};
            _grabber.y = adjustedRange.lerp(getSliderPct());
         }
         break;
         case SliderType::HORIZONTAL: {
            GFCSDraw::Vec2<double> adjustedRange = {0, _rect.get().width - _grabber.width};
            _grabber.x = adjustedRange.lerp(getSliderPct());
         }
         break;
      }
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
   friend class ScrollArea;
};