#include "LineEdit.h"
#include "SystemTime.h"

using namespace std;
using namespace ReyEngine;

///////////////////////////////////////////////////////////////////////////////////////
void LineEdit::render2D() const {
   ScopeScissor scissor(getGlobalTransform(true), getSizeRect());
   bool disabled = !getIsEnabled();
   drawRectangle(getRect().toSizeRect(), disabled ? theme->background.colorDisabled : theme->background.colorTertiary);

   auto& font = theme->font;
   auto textheight = font->size;
   //available vertical height
   float availableHeight= getRect().height;
   float textPosV = ((availableHeight - textheight) / 2);

   static constexpr int textStartHPos = 1;
   //draw default text
   auto displayText = _input.empty() ? _defaultText : _input;
   bool isDefaultText = _input.empty();
   if (!displayText.empty() && (!isDefaultText || !_isEditing)) {
      if (_highlight_start || _highlight_end) {
         auto highlightRect = getRect() - Size<R_FLOAT>(2, 2) + Pos<R_FLOAT>(1, 1);
         drawRectangle(highlightRect, theme->highlight.colorPrimary);
      }
      auto textWidth = measureText(displayText, font).x;
      if (textWidth <= 0) return;
      auto ourWidth = getWidth();
      auto textStart = textWidth > ourWidth ? ourWidth - textWidth : 2;

      ColorRGBA textColor;
      if (!_input.empty() && !disabled){
         textColor = font->color; //normal text, not disabled
      }  else if (!_input.empty() && disabled){
         textColor = font->color; //normal text, disabled
      } else if (!_defaultText.empty()){
         textColor = font->colorDisabled; //default text
      }
      drawText(displayText, {textStart, textPosV}, font, textColor, font->size, font->spacing);
   }

   //draw caret
   if (_isEditing) {
      auto frameCounter = getEngineFrameCount();
      auto caretHigh = frameCounter % 60 > 30;
      if (caretHigh) {
         //measure the position of the text for where it should start
         float caretHPos = 80;
         //set caret pos
         auto substr = _caretPos == -1 ? _input : _input.substr(0, _caretPos);
         caretHPos = measureText(substr, theme->font).x + 4;
         if (caretHPos > getWidth()){
            caretHPos = getWidth() - 2;
         }
         drawLine({{caretHPos, 2}, {caretHPos, getHeight() - 2}}, 2, theme->font->color);
      }
   }
   drawRectangleLines(getSizeRect(), 1.0, theme->background.colorPrimary);
}

///////////////////////////////////////////////////////////////////////////////////////
void LineEdit::setDefaultText(const std::string& _newDefaultText, bool noPublish) {
   auto oldText = _defaultText;
   _defaultText = _newDefaultText;
   _on_default_text_changed(oldText, _defaultText);
   if (!noPublish) {
      EventLineEditDefaultTextChanged event(this, oldText, _defaultText);
      publish(event);
   }
}

///////////////////////////////////////////////////////////////////////////////////////
void LineEdit::setText(const std::string& _newText, bool noPublish) {
   auto oldText = _input;
   _input = _newText;
   _on_text_changed(oldText, _input);
   if (!noPublish) publishText(oldText);
}

///////////////////////////////////////////////////////////////////////////////////////
Widget* LineEdit::_unhandled_input(const InputEvent& event) {
   if (auto isMouse = event.isMouse()){
      auto& mouse = isMouse.value();
      switch (event.eventId) {
         case InputEventMouseButton::ID:
            const auto& mouseEvent = event.toEvent<InputEventMouseButton>();
            if (getIsEnabled() && mouse->isInside()) {
               if (!mouseEvent.isDown) {
                  Logger::info() << getName() << endl;
                  if (isFocused()){
                     //has input - move caret
                     //see if we clicked off the end
                     if (mouse->getLocalPos().x > measureText(_input, theme->font).x){
                        //clicked off end - set caret position to the end
                        _caretPos = -1;
                        return this;
                     };
                  } else {
                     //grab iput
                     setFocused(true);
                     _caretPos = -1;
                  }
               }
               if (isFocused() && !_input.empty()){
                  //which character did we click on?
                  if (auto valid = getSubstrInfoAt(_input, mouse->getLocalPos(), theme->font)) {
                     auto [index, substr] = valid.value();
                     Logger::info() << "Clicked on character " << substr.back() << " at position " << index << ", substr = " << substr << endl;
                     _caretPos = (int)index;
                  }
               }
               //eat input
               return this;
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
            if (!isFocused()) break;
            const auto& charEvent = event.toEvent<InputEventChar>();
            auto oldText = _input;
            size_t insertPos = (_caretPos == -1) ? _input.size() : (size_t)_caretPos;
            _input.insert(insertPos, 1, charEvent.ch);
            _caretPos = (int)insertPos + 1;
            publishText(oldText);
            return this;
         }
         case InputEventKey::getUniqueEventId(): {
            const auto& keyEvent = event.toEvent<InputEventKey>();
            if (!isFocused()) break;
            switch (keyEvent.key) {
               default: break;
               case InputInterface::KeyCode::KEY_RIGHT: if (!_input.empty()) _caretPos += 1; if (_caretPos > _input.size()) return this;
               case InputInterface::KeyCode::KEY_LEFT: if (!_input.empty()) _caretPos -= 1; if (_caretPos < 0) _caretPos = -1; return this;
               case InputInterface::KeyCode::KEY_DELETE:
               case InputInterface::KeyCode::KEY_BACKSPACE:
                  if (keyEvent.isDown) {
                     size_t erasePos = (_caretPos == -1) ? _input.size() : (size_t)_caretPos;
                     if (!_input.empty() && erasePos > 0) {
                        auto oldText = _input;
                        if (keyEvent.key == InputInterface::KeyCode::KEY_DELETE){
                           erasePos += 1;
                        } else {
                           erasePos -= 1;
                        }
                        _input.erase(erasePos - 1, 1);
                        _caretPos = (int)erasePos - 1;
                        publishText(oldText);
                     }
                     return this;
                  }
               break;
               case InputInterface::KeyCode::KEY_ENTER:
                  if (keyEvent.isDown){
                     publish(EventLineEditTextEntered(this));
                     return this;
                  }
                  break;
            }
         }
      }
   }
   return nullptr;
}

///////////////////////////////////////////////////////////////////////////////////////
void LineEdit::_init() {
   setMaxSize(std::numeric_limits<float>::max(), measureText(_defaultText, theme->font).y);
}

///////////////////////////////////////////////////////////////////////////////////////
void LineEdit::_on_focus_gained() {
   if (!_isEditing) {
      _isEditing = true;
      _caretPos = -1;
   }
}
///////////////////////////////////////////////////////////////////////////////////////
void LineEdit::_on_focus_lost() {
   _isEditing = false;
}

///////////////////////////////////////////////////////////////////////////////////////
void LineEdit::publishText(const std::string& oldText) {
   EventLineEditTextChanged event(this, oldText, _input);
   publish(event);
}

///////////////////////////////////////////////////////////////////////////////////////
void LineEdit::_on_rect_changed() {

}