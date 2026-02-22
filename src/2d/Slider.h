#pragma once
#include "Widget.h"
#include "MathUtils.h"

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
         _compute_appearance();
      }
      // Set the visible amount (page size) - determines grabber size
      // When visibleAmount >= range, grabber fills the slider
      // When visibleAmount is small relative to range, grabber is small (down to minimum)
      void setVisibleAmount(double amount) {
         _visibleAmount = amount;
         _compute_appearance();
      }
      double getVisibleAmount() const { return _visibleAmount; }
      void setMinGrabberSize(float size) { _minGrabberSize = size; }
      inline float getSliderValue() const {return sliderValue;}
      inline void setSliderValue(float value, bool publish=true){
         sliderValue = Math::clamp(minSliderValue, maxSlidervalue, value);
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
               setFocused(true);
               // Calculate new grabber position based on mouse movement (1:1 tracking)
               // Then convert grabber position to slider value
               double newValue;
               switch (sliderType) {
                  case SliderType::VERTICAL: {
                     float newGrabberY = localPos.y - _dragOffset;
                     Vec2<double> grabberRange = {0, getHeight() - _grabber.height};
                     newValue = _range.lerp(grabberRange.pct(newGrabberY));
                     sliderValue = _range.clamp(newValue);
                  }
                  break;
                  case SliderType::HORIZONTAL: {
                     float newGrabberX = localPos.x - _dragOffset;
                     Vec2<double> grabberRange = {0, getWidth() - _grabber.width};
                     newValue = _range.lerp(grabberRange.pct(newGrabberX));
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
               // Store offset from mouse to grabber position so grabber follows mouse 1:1
               switch (sliderType) {
                  case SliderType::VERTICAL:
                     _dragOffset = localPos.y - _grabber.y;
                     break;
                  case SliderType::HORIZONTAL:
                     _dragOffset = localPos.x - _grabber.x;
                     break;
               }
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
         // Calculate grabber size based on visible amount vs total range
         // ratio = visibleAmount / (range + visibleAmount)
         // This way, if visible == range, grabber takes ~50% of slider
         // If visible >> range, grabber approaches 100%
         // If visible << range, grabber approaches minimum size
         double totalRange = _range.y - _range.x;

         switch(sliderType){
            case SliderType::VERTICAL: {
               _grabber.width = getRect().width;
               if (_visibleAmount > 0 && totalRange > 0) {
                  double ratio = _visibleAmount / (totalRange + _visibleAmount);
                  float calculatedHeight = static_cast<float>(getRect().height * ratio);
                  _grabber.height = std::max(_minGrabberSize, calculatedHeight);
               } else {
                  _grabber.height = getRect().height / 10; // default behavior
               }
               Vec2<double> adjustedRange = {0, getHeight() - _grabber.height};
               _grabber.y = adjustedRange.lerp(getSliderPct());
            }
            break;
            case SliderType::HORIZONTAL: {
               _grabber.height = getRect().height;
               if (_visibleAmount > 0 && totalRange > 0) {
                  double ratio = _visibleAmount / (totalRange + _visibleAmount);
                  float calculatedWidth = static_cast<float>(getRect().width * ratio);
                  _grabber.width = std::max(_minGrabberSize, calculatedWidth);
               } else {
                  _grabber.width = getRect().width / 10; // default behavior
               }
               Vec2<double> adjustedRange = {0, getWidth() - _grabber.width};
               _grabber.x = adjustedRange.lerp(getSliderPct());
            }
            break;
         }
      }

      float sliderValue = 0; //0 to 100
      float minSliderValue = 0;
      float maxSlidervalue = 100;
      double _visibleAmount = 0; // 0 means use default (fixed size grabber)
      float _minGrabberSize = 20.0f; // minimum grabber size in pixels
      SliderType sliderType;
      bool _cursor_in_slider = false;
      bool _cursor_in_grabber = false;
      bool _cursor_down = false;
      bool _is_dragging = false;
      float _dragOffset = 0; // offset from mouse to grabber position when drag starts
      Rect<float> _grabber = {0, 0, 0, 0};
      Vec2<float> _range = {0,0};
      Pos<float> _localPos;
      friend class ScrollArea;
   };
}