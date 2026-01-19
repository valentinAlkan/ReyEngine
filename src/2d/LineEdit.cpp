#include "LineEdit.h"
#include "SystemTime.h"

using namespace std;
using namespace ReyEngine;

///////////////////////////////////////////////////////////////////////////////////////
void LineEdit::render2D() const {
   ScopeScissor scissor(getGlobalTransform(true), getSizeRect());
   bool disabled = !getIsEnabled();
   drawRectangle(getSizeRect(), disabled ? theme->background.colorDisabled : theme->background.colorTertiary);

   auto& font = theme->font;
   static constexpr float textMargin = 4.0f;

   //draw default text
   auto displayText = _input.empty() ? _defaultText : _input;
   bool isDefaultText = _input.empty();
   auto textSize = measureText(displayText, font);
   float textStartX = textMargin - _scrollOffset;
   float textPosV = (getHeight() - textSize.y) / 2;

   if (!displayText.empty() && (!isDefaultText || !_isEditing)) {
      if (_highlight_start || _highlight_end) {
         auto highlightRect = getSizeRect().embiggen(-2);
         drawRectangle(highlightRect, theme->highlight.colorPrimary);
      }

      ColorRGBA textColor;
      if (!_input.empty() && !disabled){
         textColor = font->color; //normal text, not disabled
      }  else if (!_input.empty() && disabled){
         textColor = font->color; //normal text, disabled
      } else if (!_defaultText.empty()){
         textColor = font->colorDisabled; //default text
      }
      drawText(displayText, {textStartX, textPosV}, font, textColor, font->size, font->spacing);
   }

   //draw caret
   if (_isEditing) {
      auto frameCounter = getEngineFrameCount();
      auto caretHigh = frameCounter % 60 > 30;
      if (caretHigh) {
         auto substr = _caretPos == -1 ? _input : _input.substr(0, _caretPos);
         float caretHPos = textStartX + measureText(substr, theme->font).x;
         drawLine({{caretHPos, textPosV}, {caretHPos, textPosV + textSize.y}}, 2, theme->font->color);
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
                  if (!isFocused()) {
                     //grab input
                     setFocused(true);
                     _caretPos = -1;
                  }
               }
               if (isFocused() && !_input.empty()){
                  static constexpr float textMargin = 4.0f;
                  // Adjust click position for scroll offset
                  Pos<float> adjustedPos = {mouse->getLocalPos().x - textMargin + _scrollOffset, mouse->getLocalPos().y};
                  float textWidth = measureText(_input, theme->font).x;

                  if (adjustedPos.x >= textWidth) {
                     //clicked off end - set caret position to the end
                     _caretPos = -1;
                  } else if (adjustedPos.x <= 0) {
                     //clicked before start
                     _caretPos = 0;
                  } else if (auto valid = getSubstrInfoAt(_input, adjustedPos, theme->font)) {
                     auto [index, substr] = valid.value();
                     Logger::info() << "Clicked on character " << substr.back() << " at position " << index << ", substr = " << substr << endl;
                     _caretPos = (int)index;
                  }
                  ensureCaretVisible();
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
            ensureCaretVisible();
            return this;
         }
         case InputEventKey::getUniqueEventId(): {
            const auto& keyEvent = event.toEvent<InputEventKey>();
            if (!isFocused()) break;
            switch (keyEvent.key) {
               default: break;
               case InputInterface::KeyCode::KEY_RIGHT:
                  if (keyEvent.isDown && !_input.empty()) {
                     if (_caretPos == -1) {
                        // Already at end, do nothing
                     } else {
                        _caretPos += 1;
                        if ((size_t)_caretPos >= _input.size()) _caretPos = -1;
                     }
                     ensureCaretVisible();
                  }
                  return this;
               case InputInterface::KeyCode::KEY_LEFT:
                  if (keyEvent.isDown && !_input.empty()) {
                     if (_caretPos == -1) {
                        _caretPos = (int)_input.size() - 1;
                     } else if (_caretPos > 0) {
                        _caretPos -= 1;
                     }
                     ensureCaretVisible();
                  }
                  return this;
               case InputInterface::KeyCode::KEY_HOME:
                  if (keyEvent.isDown) {
                     _caretPos = 0;
                     ensureCaretVisible();
                  }
                  return this;
               case InputInterface::KeyCode::KEY_END:
                  if (keyEvent.isDown) {
                     _caretPos = -1;
                     ensureCaretVisible();
                  }
                  return this;
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
                        ensureCaretVisible();
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
      ensureCaretVisible();
   }
}
///////////////////////////////////////////////////////////////////////////////////////
void LineEdit::_on_focus_lost() {
   _isEditing = false;
   _scrollOffset = 0; // Reset scroll when not editing
}

///////////////////////////////////////////////////////////////////////////////////////
void LineEdit::publishText(const std::string& oldText) {
   EventLineEditTextChanged event(this, oldText, _input);
   publish(event);
}

///////////////////////////////////////////////////////////////////////////////////////
void LineEdit::_on_rect_changed() {

}

///////////////////////////////////////////////////////////////////////////////////////
void LineEdit::ensureCaretVisible() {
   static constexpr float textMargin = 4.0f;

   // Calculate caret position in text coordinates (without scroll)
   auto substr = _caretPos == -1 ? _input : _input.substr(0, _caretPos);
   float caretTextPos = measureText(substr, theme->font).x;

   // Calculate visible area
   float visibleWidth = getWidth() - (2 * textMargin);

   // Caret position on screen = caretTextPos - _scrollOffset + textMargin
   float caretScreenPos = caretTextPos - _scrollOffset + textMargin;

   // If caret is off the right edge, scroll right
   if (caretScreenPos > getWidth() - textMargin) {
      _scrollOffset = caretTextPos - visibleWidth;
   }
   // If caret is off the left edge, scroll left
   else if (caretScreenPos < textMargin) {
      _scrollOffset = caretTextPos;
   }

   // Don't scroll past the beginning
   if (_scrollOffset < 0) {
      _scrollOffset = 0;
   }
}