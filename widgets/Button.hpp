#pragma once
#include "Control.hpp"

class BaseButton : public Control {
public:
   struct PushButtonEvent : public Event<PushButtonEvent>{
      EVENT_CTOR_SIMPLE(PushButtonEvent, Event<PushButtonEvent>, bool down), down(down){}
      bool down;
   };
protected:
   GFCSDRAW_OBJECT(BaseButton, Control)
   , PROPERTY_DECLARE(down){
      _rect.value = GFCSDraw::Rect<double>(0,0,200,50);
      acceptsHover = true;
   }
public:
   void registerProperties() override{
      registerProperty(down);
   };
   BoolProperty down;
   bool wasDown;
protected:
   Handled _unhandled_input(InputEvent& event) override{
      if (_isEditorWidget) return false;
      switch (event.eventId) {
         case InputEventMouseButton::getUniqueEventId(): {
            auto mouseEvent = event.toEventType<InputEventMouseButton>();
            if (isInside(globalToLocal(mouseEvent.globalPos))) {
               if (mouseEvent.button == InputInterface::MouseButton::MOUSE_BUTTON_LEFT) {
                  setDown(mouseEvent.isDown);
                  return true;
               }
            }
         }
         break;
         case InputEventMouseMotion::getUniqueEventId():
            auto mouseEvent = event.toEventType<InputEventMouseMotion>();
            if (isInside(globalToLocal(mouseEvent.globalPos))){

            }
         break;
      }
      return false;
   }
   void setDown(bool newDown){
      if (wasDown != newDown){
         down.set(newDown);
         auto e = PushButtonEvent(toEventPublisher(), newDown);
         publish<PushButtonEvent>(e);
      }
   }
};

/////////////////////////////////////////////////////////////////////////////////////////
class PushButton : public BaseButton{
   GFCSDRAW_OBJECT(PushButton, BaseButton)
   , PROPERTY_DECLARE(text){
      text.value = getName();
      getTheme()->background.colorPrimary.set(COLORS::gray);
      getTheme()->background.colorSecondary.set(COLORS::lightGray);
      getTheme()->background.colorTertiary.set(COLORS::blue);
   }
   void registerProperties() override {
      registerProperty(text);
   };
   void render() const override {
      static constexpr int SEGMENTS = 10;
      static constexpr int THICKNESS = 2;
      auto color = getThemeReadOnly().background.colorPrimary.value;
      if (isHovered()) color = getThemeReadOnly().background.colorSecondary.value;
      if (down.value) color = getThemeReadOnly().background.colorTertiary.value;
      _drawRectangleRounded(getRect().toSizeRect(), getThemeReadOnly().roundness.get(), SEGMENTS, color);
      _drawRectangleRoundedLines(getRect().toSizeRect(), getThemeReadOnly().roundness.get(), SEGMENTS, THICKNESS, COLORS::black);
      _drawTextCentered(text.value, getRect().toSizeRect().center(), getThemeReadOnly().font.value);
   }
   StringProperty text;
public:
   std::string getText(){return text.value;}
   void setText(const std::string& newText){text.value = newText;}
protected:

};

