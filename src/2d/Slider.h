#pragma once
#include "Widget.h"

namespace ReyEngine{
   class Slider : public Widget {
   public:
      REYENGINE_OBJECT(Slider)
      EVENT(EventSliderValueChanged, 812735879612)
         {}
         double value;
         Fraction pct;
      };

      EVENT(EventSliderPressed, 812735879613)
         {}
         double value;
         Fraction pct;
      };

      EVENT(EventSliderReleased, 812735879614)
         {}
         double value;
         Fraction pct;
      };

      enum class SliderType{VERTICAL, HORIZONTAL};
      Vec2<double> getRange(){return _range;}
      void setRange(Vec2<double> newRange){
         minSliderValue = newRange.x;
         maxSlidervalue = newRange.y;
         _range = {minSliderValue, maxSlidervalue};
      }
      inline double getSliderValue() const {return sliderValue;}
      inline void setSliderValue(float value, bool publish=true){
         sliderValue = value;
         if (publish) _publish_slider_val<EventSliderValueChanged>();
         _compute_appearance();
      }
      inline Fraction getSliderPct() const {return _range.pct(sliderValue);}
      inline void setSliderPct(Fraction pct, bool publish=true){setSliderValue(_range.lerp(pct), publish);}

      Slider(SliderType sliderDir)
      : sliderType(sliderDir)
      {
         constexpr float DEFAULT_SIZE = 20;
         constexpr float MAX_SIZE = 50;
         switch (sliderDir){
            case SliderType::HORIZONTAL:
               maxSize = {std::numeric_limits<float>::max(), MAX_SIZE};
               setHeight(DEFAULT_SIZE);
               break;
            case SliderType::VERTICAL:
               maxSize = {MAX_SIZE, std::numeric_limits<float>::max()};
               setWidth(DEFAULT_SIZE);
               break;
         }
         _range = {minSliderValue, maxSlidervalue};
      }
   protected:
      Widget* _unhandled_input(const InputEvent& e) override {
         auto mouseEvent = e.isMouse();
         if (!mouseEvent) return nullptr;
         auto localPos = mouseEvent.value()->getLocalPos();
         if (e.isEvent<InputEventMouseMotion>()){
            _localPos = e.isMouse().value()->getLocalPos();
            if (_is_dragging) {
               //set new slider value based on input
               double newValue;
               switch (sliderType) {
                  case SliderType::VERTICAL: {
                     auto heightRange = ReyEngine::Vec2<double>(0, getRect().height);
                     newValue = _range.lerp(heightRange.pct(localPos.y));
                     sliderValue = _range.clamp(newValue);
                  }
                  break;
                  case SliderType::HORIZONTAL: {
                     auto widthRange = Vec2<float>(0, getWidth());
                     newValue = _range.lerp(widthRange.pct(localPos.x));
                     sliderValue = _range.clamp(newValue);
                  }
                  break;
               }
               _compute_appearance();
               _publish_slider_val<EventSliderValueChanged>();
               return this;
            }
         }

         if (e.isEvent<InputEventMouseButton>()){
            auto &buttonEvent = e.toEvent<InputEventMouseButton>();
            if (mouseEvent.value()->isInside() && buttonEvent.isDown && _grabber.contains(localPos)) {
               _cursor_down = true;
               _is_dragging = _cursor_down;
               setFocused(true);
               _publish_slider_val<EventSliderPressed>();
               return this;
            }
            if (!buttonEvent.isDown && _is_dragging) {
               _cursor_down = buttonEvent.isDown;
               _is_dragging = false;
               setFocused(false);
               _publish_slider_val<EventSliderReleased>();
               return this;
            }
         }
         return nullptr;
      }
      void render2D() const override {
         //draw slider
         auto r = getRect().toSizeRect();
         drawRectangle(r, _cursor_in_slider || _is_dragging ? Colors::green : Colors::red);
         //draw grabber
         drawRectangle(_grabber, _cursor_down && _cursor_in_grabber || _is_dragging ? Colors::yellow : Colors::blue);
         drawRectangleLines(_grabber, 1.0, Colors::black);
         drawRectangleLines(getSizeRect(), 1.0, Colors::black);
      }
      void _on_rect_changed() override {
         _compute_appearance();
      };
      void _process(float dt) override {}
   private:
      template <typename EventType>
      void _publish_slider_val(){
         auto event = EventType(this);
         event.value = getSliderValue();
         event.pct = Fraction(getSliderPct()).get();
         publish<EventType>(event);
      }
      void _compute_appearance(){
         switch(sliderType){
            case SliderType::VERTICAL: {
               _grabber.width= getRect().width;
               _grabber.height= getRect().height / 10;
               Vec2<double> adjustedRange = {0, getHeight() - _grabber.height};
               _grabber.y = adjustedRange.lerp(getSliderPct());
            }
            break;
            case SliderType::HORIZONTAL: {
               _grabber.width= getRect().width/10;
               _grabber.height= getRect().height;
               Vec2<double> adjustedRange = {0, getWidth() - _grabber.width};
               _grabber.x = adjustedRange.lerp(getSliderPct());
            }
            break;
         }
      }

      float sliderValue = 0; //0 to 100
      float minSliderValue = 0;
      float maxSlidervalue = 100;
      SliderType sliderType;
      bool _cursor_in_slider = false;
      bool _cursor_in_grabber = false;
      bool _cursor_down = false;
      bool _is_dragging = false;
      Rect<float> _grabber = {0, 0, 0, 0};
      Vec2<float> _range = {0,0};
      Pos<float> _localPos;
      friend class ScrollArea;
   };
}