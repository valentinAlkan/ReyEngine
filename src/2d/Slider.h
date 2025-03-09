#pragma once
#include "Widget.h"

namespace ReyEngine{
   class Slider : public Widget {
   public:
      REYENGINE_OBJECT(Slider)
      EVENT(EventSliderValueChanged, 812735879612)
         {}
         double value;
         Perunum pct;
      };

      enum class SliderType{VERTICAL, HORIZONTAL};
      Vec2<double> getRange(){return _range;}
      void setRange(Vec2<double> newRange){
         minSliderValue = newRange.x;
         maxSlidervalue = newRange.y;
         _range = {minSliderValue, maxSlidervalue};
      }
      inline double getSliderValue() const {return sliderValue;}
      inline void setSliderValue(float value, bool publish=true){sliderValue = value; if (publish)_publish_slider_val();_compute_appearance();}
      inline Perunum getSliderPct() const {return _range.pct(sliderValue);}
      inline void setSliderPct(Perunum pct, bool publish=true){setSliderValue(_range.lerp(pct), publish);}

      Slider(SliderType sliderDir)
      : sliderType(sliderDir)
      {
         constexpr float MAX_SIZE = 50;
         switch (sliderDir){
            case SliderType::HORIZONTAL:
               maxSize = {std::numeric_limits<decltype(size.x)>::max(), MAX_SIZE};
               break;
            case SliderType::VERTICAL:
               maxSize = {MAX_SIZE, std::numeric_limits<decltype(size.x)>::max()};
               break;
         }
         _range = {minSliderValue, maxSlidervalue};
      }
   protected:
      Handled _unhandled_input(const InputEvent& e) override {
         auto mouseEvent = e.isMouse();
         if (!mouseEvent) return false;
         auto localPos = mouseEvent.value()->getLocalPos();
         if (e.isEvent<InputEventMouseMotion>()){
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
                  case SliderType::HORIZONTAL:
                     auto widthRange = Vec2<float>(0, getWidth());
                     newValue = _range.lerp(widthRange.pct(localPos.x));
                     sliderValue = _range.clamp(newValue);
                  _compute_appearance();
                  _publish_slider_val();
               }
               return true;
            }
         }

         if (e.isEvent<InputEventMouseButton>()){
            auto &buttonEvent = e.toEvent<InputEventMouseButton>();
            if (mouseEvent.value()->isInside() && buttonEvent.isDown && _grabber.isInside(localPos)) {
               _cursor_down = true;
               _is_dragging = _cursor_down;
               return true;
            }
            if (!buttonEvent.isDown && _is_dragging) {
               _cursor_down = buttonEvent.isDown;
               _is_dragging = false;
               return true;
            }
         }
         return false;
      }
      void render2D() const override {
         //draw slider
         auto r = getRect().toSizeRect();
         drawRectangle(r, _cursor_in_slider || _is_dragging ? Colors::green : Colors::red);
         //draw grabber
         drawRectangle(_grabber, _cursor_down && _cursor_in_grabber || _is_dragging ? Colors::yellow : Colors::blue);
         if constexpr (false) {
            //debug
            auto x = Vec2<float>(0, getWidth());
            auto y = Vec2<float>(0, getHeight());
            auto s = getSliderValue();
            Circle c({x.lerp(Percent(s)), y.lerp(Percent(s))}, 2);
            drawCircle(c, Colors::black);
         }
      }
      void _on_rect_changed() override {
         _compute_appearance();
      };
      void _process(float dt) override {}
   private:
      void _publish_slider_val(){
         auto event = EventSliderValueChanged(this);
         event.value = getSliderValue();
         event.pct = Perunum(getSliderPct()).get();
         publish<EventSliderValueChanged>(event);
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
      friend class ScrollArea;
   };
}