#pragma once

#include "Control.h"

namespace ReyEngine{
   class Slider : public BaseWidget {
      REYENGINE_OBJECT_CUSTOM_BUILD(Slider, BaseWidget);
   public:
      class EventSliderValueChanged : public Event<EventSliderValueChanged> {
      public:
         EVENT_CTOR_SIMPLE(EventSliderValueChanged, Event<EventSliderValueChanged>){}
         double value;
         Perunum pct;
      };

      enum class SliderType{VERTICAL, HORIZONTAL};
      struct SliderTypeProperty : public Property<SliderType>{
         SliderTypeProperty(const std::string& instanceName, SliderType defaultvalue = SliderType::VERTICAL)
         : Property(instanceName, PropertyTypes::Float, std::move(defaultvalue))
         {}
         std::string toString() const override {return value == SliderType::VERTICAL ? "VERTICAL" : "HORIZONTAL";}
         SliderType fromString(const std::string& str) override {return (str == "VERTICAL" ? SliderType::VERTICAL : SliderType::HORIZONTAL);}
      };

      static std::shared_ptr<Slider> build(const std::string &name, SliderType sliderType) noexcept {
         auto me = _reyengine_make_shared(name, sliderType);
         return me;
      }

      ReyEngine::Vec2<double> getRange(){return _range;}
      void setRange(ReyEngine::Vec2<double> newRange){
         minSliderValue.set(newRange.x);
         maxSlidervalue.set(newRange.y);
         _range = {minSliderValue.value, maxSlidervalue.value};
      }
      inline double getSliderValue(){return sliderValue.value;}
      inline void setSliderValue(float value, bool publish=true){sliderValue.set(value);if (publish)_publish_slider_val();_compute_appearance();}
      inline Perunum getSliderPct(){return _range.pct(sliderValue.value);}
      inline void setSliderPct(Perunum pct, bool publish=true){setSliderValue(_range.lerp(pct), publish);}

      Slider(const std::string &name, SliderType sliderDir)
      : BaseWidget(name, _get_static_constexpr_typename())
      , NamedInstance(name, _get_static_constexpr_typename())
      , Component(name, _get_static_constexpr_typename())
      , sliderValue("sliderValue", 0.0)
      , minSliderValue("minSliderValue", 0.0)
      , maxSlidervalue("maxSliderValue", 100.0)
      , sliderType("sliderType", sliderDir)
      {
         _range = {minSliderValue.value, maxSlidervalue.value};
      }
   protected:
      Handled _unhandled_input(const InputEvent& e, const std::optional<UnhandledMouseInput>& mouse) override {
         auto& mouseEvent = (InputEventMouseMotion&)(e);
         auto localPos = globalToLocal(mouseEvent.globalPos);
         if (mouseEvent.isEvent<InputEventMouseMotion>()){
            if (_is_dragging) {
               //drag grabber
               if (_is_dragging) {
                  //set new slider value based on input
                  double newValue = 0;
                  switch (sliderType.value) {
                     case SliderType::VERTICAL: {
                        auto heightRange = ReyEngine::Vec2<double>(0, getRect().height);
                        newValue = _range.lerp(heightRange.pct(localPos.y));
                        sliderValue.set(_range.clamp(newValue));
                     }
                        break;
                     case SliderType::HORIZONTAL:
                        auto widthRange = ReyEngine::Vec2<double>(0, getRect().width);
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
         return false;
      }
      void render() const override {
         //draw slider
         drawRectangle(getRect().toSizeRect(), _cursor_in_slider || _is_dragging? ReyEngine::Colors::green : ReyEngine::Colors::red);
         //draw grabber
         drawRectangle(_grabber, _cursor_down && _cursor_in_grabber || _is_dragging ? ReyEngine::Colors::yellow : ReyEngine::Colors::blue);
      }
      void _on_rect_changed() override {
         _compute_appearance();
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
         auto event = EventSliderValueChanged(EventPublisher::shared_from_this());
         event.value = getSliderValue();
         event.pct = Perunum(getSliderPct()).get();
         publish<EventSliderValueChanged>(event);
      }
      void _compute_appearance(){
         switch(sliderType.value){
            case SliderType::VERTICAL: {
               _grabber.width= getRect().width;
               _grabber.height= getRect().height / 10;
               ReyEngine::Vec2<double> adjustedRange = {0, getHeight() - _grabber.height};
               _grabber.y = adjustedRange.lerp(getSliderPct());
            }
            break;
            case SliderType::HORIZONTAL: {
               _grabber.width= getRect().width/10;
               _grabber.height= getRect().height;
               ReyEngine::Vec2<double> adjustedRange = {0, getWidth() - _grabber.width};
               _grabber.x = adjustedRange.lerp(getSliderPct());
            }
            break;
         }
      }

      FloatProperty sliderValue; //0 to 1
      FloatProperty minSliderValue;
      FloatProperty maxSlidervalue;
      SliderTypeProperty sliderType;
      bool _cursor_in_slider = false;
      bool _cursor_in_grabber = false;
      bool _cursor_down = false;
      bool _is_dragging = false;
      ReyEngine::Rect<double> _grabber = {0, 0, 0, 0};
      ReyEngine::Vec2<double> _range = {0,0};
      friend class ScrollArea;
   };
}