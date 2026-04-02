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
      Handled _unhandled_input(const InputEvent& e) override {
         auto mouseEvent = e.isMouse();
         if (!mouseEvent) return nullptr;
         auto localPos = mouseEvent.value()->getLocalPos();
         if (e.isEvent<InputEventMouseMotion>()){
            if (_is_dragging) {
               setFocused(true);
               // Calculate new grabber position based on mouse movement (1:1 tracking)
               // Then convert grabber position to slider value
               double newValue;
               switch (sliderType) {
                  case SliderType::VERTICAL: {
                     float newGrabberY = localPos.y - _dragOffset;
                     // Position is relative to track, not full slider
                     Vec2<double> grabberRange = {_track.y, _track.y + _track.height - _grabber.height};
                     newValue = _range.lerp(grabberRange.pct(newGrabberY));
                     sliderValue = _range.clamp(newValue);
                  }
                  break;
                  case SliderType::HORIZONTAL: {
                     float newGrabberX = localPos.x - _dragOffset;
                     // Position is relative to track, not full slider
                     Vec2<double> grabberRange = {_track.x, _track.x + _track.width - _grabber.width};
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

         if (e.isEvent<InputEventMouseButton>()) {
            auto &buttonEvent = e.toEvent<InputEventMouseButton>();
            if (mouseEvent.value()->isInside() && buttonEvent.isDown) {
               // Check nudge buttons first
               if (_btnNudge0.contains(localPos)) {
                  _cursorInNudge0 = true;
                  setSliderValue(sliderValue - _nudgeAmount);
                  return this;
               }
               if (_btnNudge1.contains(localPos)) {
                  _cursorInNudge1 = true;
                  setSliderValue(sliderValue + _nudgeAmount);
                  return this;
               }
               // Check grabber for dragging
               if (_grabber.contains(localPos)) {
                  _cursorInGrabber = true;
                  _is_dragging = true;
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
            }
            if (!buttonEvent.isDown) {
               _cursorInGrabber = false;
               _cursorInNudge0 = false;
               _cursorInNudge1 = false;
               if (_is_dragging) {
                  _is_dragging = false;
                  setFocused(false);
                  _publish_slider_val<EventSliderReleased>();
                  return this;
               }
            }
         }
         return nullptr;
      }
      void render2D(RenderContext&) const override {
         //draw nudge buttons
         static constexpr float ROUNDNESS = 0.3;
         static constexpr float SEGMENTS = 10;
         auto nudgerect0 = _btnNudge0.embiggen(-2);
         auto nudgerect1 = _btnNudge1.embiggen(-2);
         const auto& active1 = getTheme().foreground.colorActive1;
         const auto& active2 = getTheme().foreground.colorActive2;
         drawRectangleRounded(nudgerect0, ROUNDNESS, SEGMENTS, _cursorInNudge0 ? active2 : active1);
         drawRectangleRounded(nudgerect1, ROUNDNESS, SEGMENTS, _cursorInNudge1 ? active2 : active1);
         //draw grabber
         drawRectangleRounded(_grabber.embiggen(-4), ROUNDNESS, SEGMENTS, _cursorInGrabber || _is_dragging ? active2 : active1);
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
               float btnSize = getRect().width; // square buttons
               // Nudge buttons at top and bottom
               _btnNudge0 = {0, 0, btnSize, btnSize};
               _btnNudge1 = {0, getHeight() - btnSize, btnSize, btnSize};
               // Track is the area between buttons
               _track = {0, btnSize, getRect().width, getHeight() - 2 * btnSize};

               _grabber.width = _track.width;
               if (_visibleAmount > 0 && totalRange > 0) {
                  double ratio = _visibleAmount / (totalRange + _visibleAmount);
                  float calculatedHeight = static_cast<float>(_track.height * ratio);
                  _grabber.height = std::max(_minGrabberSize, calculatedHeight);
               } else {
                  _grabber.height = _track.height / 10; // default behavior
               }
               Vec2<double> adjustedRange = {0, _track.height - _grabber.height};
               _grabber.x = _track.x;
               _grabber.y = _track.y + adjustedRange.lerp(getSliderPct());
            }
            break;
            case SliderType::HORIZONTAL: {
               float btnSize = getRect().height; // square buttons
               // Nudge buttons at left and right
               _btnNudge0 = {0, 0, btnSize, btnSize};
               _btnNudge1 = {getWidth() - btnSize, 0, btnSize, btnSize};
               // Track is the area between buttons
               _track = {btnSize, 0, getWidth() - 2 * btnSize, getRect().height};

               _grabber.height = _track.height;
               if (_visibleAmount > 0 && totalRange > 0) {
                  const double ratio = _visibleAmount / (totalRange + _visibleAmount);
                  float calculatedWidth = static_cast<float>(_track.width * ratio);
                  _grabber.width = std::max(_minGrabberSize, calculatedWidth);
               } else {
                  _grabber.width = _track.width / 10; // default behavior
               }
               Vec2<double> adjustedRange = {0, _track.width - _grabber.width};
               _grabber.x = _track.x + adjustedRange.lerp(getSliderPct());
               _grabber.y = _track.y;
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
      bool _cursorInGrabber = false;
      bool _cursorInNudge0 = false;
      bool _cursorInNudge1 = false;
      bool _is_dragging = false;
      float _dragOffset = 0; // offset from mouse to grabber position when drag starts
      Rect<float> _grabber = {0, 0, 0, 0};
      Rect<float> _btnNudge0 = {0, 0, 0, 0}; // left/up nudge button
      Rect<float> _btnNudge1 = {0, 0, 0, 0}; // right/down nudge button
      Rect<float> _track = {0, 0, 0, 0}; // area where grabber can move (between nudge buttons)
      static constexpr float _nudgeAmount = 5.0f; // how much to nudge per click (in slider value units)
      Vec2<float> _range = {0,0};
      friend class ScrollArea;
   };
}