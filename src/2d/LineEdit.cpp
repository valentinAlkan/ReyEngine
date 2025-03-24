#include "LineEdit.h"
#include "SystemTime.h"

using namespace std;
using namespace ReyEngine;

///////////////////////////////////////////////////////////////////////////////////////
void LineEdit::render2D() const {
   drawRectangle(getRect().toSizeRect(), theme->background.colorTertiary);

   auto& font = theme->font;
   auto textheight = font.size;
   //available vertical height
   float availableHeight= getRect().height;
   float textPosV = ((availableHeight - textheight) / 2);

   static constexpr int textStartHPos = 1;
   auto renderText = [&](const std::string& text){
      auto textWidth = measureText(text, font).x;
      if (textWidth <= 0) return;
      auto ourWidth = getWidth();
      auto textStart = textWidth > ourWidth ? ourWidth - textWidth : 2;

//      startScissor(_scissorArea);
      drawText(text, {textStart, textPosV}, font);
//      stopScissor();
   };

   //draw default text
   if (_text.empty() && !_defaultText.empty()) {
      if (_highlight_start || _highlight_end) {
         auto highlightRect= getRect() - Size<R_FLOAT>(2, 2) + Pos<R_FLOAT>(1, 1);
         drawRectangle(highlightRect, theme->highlight.colorPrimary);
      }
      renderText(_defaultText);
   } else {
      renderText(_text);
   }

   //draw caret
   if (_isEditing) {
      auto frameCounter = getFrameCount();
      auto caretHigh = frameCounter % 60 > 30;
      if (caretHigh) {
         //measure the position of the text for where it should start
         float caretHPos;
         if (_caretPos == -1){
            //set caret to end
            caretHPos = measureText(_text, theme->font).x;
            if (caretHPos > getWidth()){
               caretHPos = getWidth() - 2;
            }
         }
         drawLine({{caretHPos, 2}, {caretHPos, getHeight() - 2}}, 2, theme->font.color);
      }
   }
//   if (_isModal && getFrameCounter() % 60 == 0){
//      cout << _caretPos << endl;
//   }
}

///////////////////////////////////////////////////////////////////////////////////////
void LineEdit::setDefaultText(const std::string& _newDefaultText, bool noPublish) {
   _defaultText = _newDefaultText;
   _on_default_text_changed(_defaultText);
   if (!noPublish) {
      EventLineEditDefaultTextChanged event(this, _defaultText);
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
Widget* LineEdit::_unhandled_input(const InputEvent& event) {
   if (auto isMouse = event.isMouse()){
      auto& mouse = isMouse.value();
      switch (event.eventId) {
         case InputEventMouseButton::ID:
            const auto& mouseEvent = event.toEvent<InputEventMouseButton>();
            if (mouse->isInside()) {
               if (!mouseEvent.isDown) {
                  if (isFocused()){
                     //has input - move caret
                     //see if we clicked off the end
                     if (mouse->getLocalPos().x > measureText(_text, theme->font).x){
                        //clicked off end - set caret position to the end
                        _caretPos = -1;
                     }
                  } else {
                     //grab iput
                     setFocused(true);
                     _caretPos = -1;
                     return this;
                  }
               }
            } else {
               if (isFocused()) {
                  //release input
                  setFocused(false);
               }
            }
            break;
      }
   }
   if (_isEditing){
      switch (event.eventId) {
         case InputEventChar::getUniqueEventId(): {
            const auto& charEvent = event.toEvent<InputEventChar>();
            _text += charEvent.ch;
            publishText();
            return this;
         }
         case InputEventKey::getUniqueEventId(): {
            const auto& keyEvent = event.toEvent<InputEventKey>();
            if (keyEvent.isDown && keyEvent.key == InputInterface::KeyCode::KEY_BACKSPACE) {
               if (!_text.empty()) {
                  _text.pop_back();
                  publishText();
               }
               return this;
            }
         }
      }
   }
   return nullptr;
}
///////////////////////////////////////////////////////////////////////////////////////
void LineEdit::_on_focus_gained() {
   _isEditing = true;
}
///////////////////////////////////////////////////////////////////////////////////////
void LineEdit::_on_focus_lost() {
   _isEditing = false;
}

///////////////////////////////////////////////////////////////////////////////////////
void LineEdit::publishText() {
   EventLineEditTextChanged event(this, _text);
   publish(event);
}

///////////////////////////////////////////////////////////////////////////////////////
void LineEdit::_on_rect_changed() {
   _scissorArea = getRect().toSizeRect();
}