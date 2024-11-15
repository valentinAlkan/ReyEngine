#include "LineEdit.h"
#include "Application.h"

using namespace std;
using namespace ReyEngine;

///////////////////////////////////////////////////////////////////////////////////////
void LineEdit::render() const {
   drawRectangle(_rect.value.toSizeRect(), theme->background.colorTertiary);

   auto& font = theme->font;
   auto textheight = font.value.size;
   //available vertical height
   auto availableHeight = _rect.value.height;
   auto textPosV = (int)((availableHeight - textheight) / 2);

   static constexpr int textStartHPos = 1;
   auto renderText = [&](const std::string& text){
      auto textWidth = measureText(text, font).x;
      if (textWidth <= 0) return;
      auto ourWidth = getWidth();
      auto textStart = textWidth > ourWidth ? ourWidth - textWidth : 2;

      startScissor(_scissorArea);
      drawText(text, {textStart, textPosV}, font);
      stopScissor();
   };

   //draw default text
   if (_text.value.empty() && !_defaultText.value.empty()) {
      if (_highlight_start || _highlight_end) {
         auto highlightRect = _rect.value - Size<R_FLOAT>(2, 2) + Pos<R_FLOAT>(1, 1);
         drawRectangle(highlightRect, theme->highlight);
      }
      renderText(_defaultText);
   } else {
      renderText(_text);
   }

   //draw caret
   if (_isEditing) {
      auto frameCounter = getFrameCounter();
      auto caretHigh = frameCounter % 60 > 30;
      if (caretHigh) {
         //measure the position of the text for where it should start
         int caretHPos;
         if (_caretPos == -1){
            //set caret to end
            caretHPos = measureText(_text.value, theme->font).x;
            if (caretHPos > getWidth()){
               caretHPos = getWidth() - 2;
            }
         }
         drawLine({{caretHPos, 2}, {caretHPos, getHeight() - 2}}, 2, theme->font.value.color);
      }
   }
//   if (_isModal && getFrameCounter() % 60 == 0){
//      cout << _caretPos << endl;
//   }
}

///////////////////////////////////////////////////////////////////////////////////////
void LineEdit::registerProperties() {
   registerProperty(_defaultText);
   registerProperty(_text);
}

///////////////////////////////////////////////////////////////////////////////////////
void LineEdit::setDefaultText(const std::string& _newDefaultText, bool noPublish) {
   _defaultText = _newDefaultText;
   _on_default_text_changed(_defaultText);
   if (!noPublish) {
      EventLineEditDefaultTextChanged event(toEventPublisher(), _defaultText);
      publish(event);
   }
}

///////////////////////////////////////////////////////////////////////////////////////
void LineEdit::setText(const std::string& _newText, bool noPublish) {
   _text = _newText;
   _on_text_changed(_text);
   if (!noPublish) publishText();
}

///////////////////////////////////////////////////////////////////////////////////////
Handled LineEdit::_unhandled_input(const InputEvent& event, const std::optional<UnhandledMouseInput>& mouse) {
   if (mouse){
      switch (event.eventId) {
         case InputEventMouseButton::getUniqueEventId():
            const auto& mouseEvent = event.toEventType<InputEventMouseButton>();
            if (mouse->isInside) {
               if (!mouseEvent.isDown) {
                  if (isModal()){
                     //has input - move caret
                     //see if we clicked off the end
                     if (mouse.value().localPos.x > measureText(_text.value, theme->font).x){
                        //clicked off end - set caret position to the end
                        _caretPos = -1;
                     }
                  } else {
                     //grab iput
                     setModal(true);
                     _caretPos = -1;
                     return true;
                  }
               }
            } else {
               if (isModal()) {
                  //release input
                  setModal(false);
               }
            }
            break;
      }
   }
   if (_isEditing){
      switch (event.eventId) {
         case InputEventChar::getUniqueEventId(): {
            const auto& charEvent = event.toEventType<InputEventChar>();
            _text.value += charEvent.ch;
            publishText();
            return true;
         }
         case InputEventKey::getUniqueEventId(): {
            const auto& keyEvent = event.toEventType<InputEventKey>();
            if (keyEvent.isDown && keyEvent.key == InputInterface::KeyCode::KEY_BACKSPACE) {
               if (!_text.value.empty()) {
                  _text.value.pop_back();
                  publishText();
               }
               return true;
            }
         }
      }
   }
   return false;
}
///////////////////////////////////////////////////////////////////////////////////////
void LineEdit::_on_modality_gained() {
   _isEditing = true;
}
///////////////////////////////////////////////////////////////////////////////////////
void LineEdit::_on_modality_lost() {
   _isEditing = false;
}

///////////////////////////////////////////////////////////////////////////////////////
void LineEdit::publishText() {
   EventLineEditTextChanged event(toEventPublisher(), _text.value);
   publish(event);
}

///////////////////////////////////////////////////////////////////////////////////////
void LineEdit::_on_rect_changed() {
   _scissorArea = getRect().toSizeRect();
}