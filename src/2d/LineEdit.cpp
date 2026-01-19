#include "LineEdit.h"
#include "SystemTime.h"
#include <cstring>

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
      // Draw selection highlight
      if (hasSelection() && !_input.empty()) {
         int minSel = getSelectionMin();
         int maxSel = getSelectionMax();
         float selStartX = textStartX + measureText(_input.substr(0, minSel), font).x;
         float selEndX = textStartX + measureText(_input.substr(0, maxSel), font).x;
         Rect<float> selRect = {{selStartX, textPosV}, {selEndX - selStartX, textSize.y}};
         drawRectangle(selRect, theme->foreground.colorHighlight);
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
int LineEdit::getCaretPosFromMouse(float mouseLocalX) const {
   static constexpr float textMargin = 4.0f;
   float adjustedX = mouseLocalX - textMargin + _scrollOffset;
   float textWidth = measureText(_input, theme->font).x;

   if (adjustedX >= textWidth) {
      return -1; // End of text
   } else if (adjustedX <= 0) {
      return 0; // Beginning of text
   } else if (auto valid = getCharIndexAt(_input, {adjustedX, 0}, theme->font)) {
      return (int)valid.value();
   }
   return -1;
}

///////////////////////////////////////////////////////////////////////////////////////
Widget* LineEdit::_unhandled_input(const InputEvent& event) {
   if (auto isMouse = event.isMouse()){
      auto& mouse = isMouse.value();
      switch (event.eventId) {
         case InputEventMouseButton::ID: {
            const auto& mouseEvent = event.toEvent<InputEventMouseButton>();
            if (getIsEnabled() && mouse->isInside()) {
               if (mouseEvent.isDown) {
                  // Mouse button pressed - start selection
                  if (!isFocused()) {
                     setFocused(true);
                  }
                  if (!_input.empty()) {
                     _caretPos = getCaretPosFromMouse(mouse->getLocalPos().x);
                     clearSelection();
                     setSelectionFromCaret();
                     _isDragging = true;
                     ensureCaretVisible();
                  }
               } else {
                  // Mouse button released - stop dragging
                  _isDragging = false;
               }
               return this;
            } else {
               if (isFocused() && !mouseEvent.isDown) {
                  // Clicked outside while focused - release focus
                  setFocused(false);
                  _isDragging = false;
               }
            }
            break;
         }
         case InputEventMouseMotion::ID: {
            if (_isDragging && isFocused() && !_input.empty()) {
               // Update selection while dragging
               _caretPos = getCaretPosFromMouse(mouse->getLocalPos().x);
               _selectionEnd = caretPosToIndex(_caretPos);
               ensureCaretVisible();
               return this;
            }
            break;
         }
      }
   }
   if (_isEditing){
      switch (event.eventId) {
         case InputEventChar::getUniqueEventId(): {
            if (!isFocused()) break;
            const auto& charEvent = event.toEvent<InputEventChar>();
            auto oldText = _input;

            // Delete selection first if any
            if (hasSelection()) {
               int minPos = getSelectionMin();
               _input.erase(minPos, getSelectionMax() - minPos);
               _caretPos = minPos;
               clearSelection();
            }

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

            bool shiftHeld = InputInterface::isKeyDown(InputInterface::KeyCode::KEY_LEFT_SHIFT) ||
                            InputInterface::isKeyDown(InputInterface::KeyCode::KEY_RIGHT_SHIFT);
            bool ctrlHeld = InputInterface::isKeyDown(InputInterface::KeyCode::KEY_LEFT_CONTROL) ||
                           InputInterface::isKeyDown(InputInterface::KeyCode::KEY_RIGHT_CONTROL);

            switch (keyEvent.key) {
               default: break;
               case InputInterface::KeyCode::KEY_A:
                  // Ctrl+A: Select all
                  if (keyEvent.isDown && ctrlHeld && !_input.empty()) {
                     _selectionStart = 0;
                     _selectionEnd = (int)_input.size();
                     _caretPos = -1;
                     ensureCaretVisible();
                  }
                  return this;
               case InputInterface::KeyCode::KEY_C:
                  // Ctrl+C: Copy
                  if (keyEvent.isDown && ctrlHeld && hasSelection()) {
                     SetClipboardText(getSelectedText().c_str());
                  }
                  return this;
               case InputInterface::KeyCode::KEY_X:
                  // Ctrl+X: Cut
                  if (keyEvent.isDown && ctrlHeld && hasSelection()) {
                     SetClipboardText(getSelectedText().c_str());
                     deleteSelection();
                  }
                  return this;
               case InputInterface::KeyCode::KEY_V:
                  // Ctrl+V: Paste
                  if (keyEvent.isDown && ctrlHeld) {
                     const char* clipText = GetClipboardText();
                     if (clipText && clipText[0] != '\0') {
                        auto oldText = _input;
                        // Delete selection first if any
                        if (hasSelection()) {
                           int minPos = getSelectionMin();
                           _input.erase(minPos, getSelectionMax() - minPos);
                           _caretPos = minPos;
                           clearSelection();
                        }
                        size_t insertPos = (_caretPos == -1) ? _input.size() : (size_t)_caretPos;
                        _input.insert(insertPos, clipText);
                        _caretPos = (int)(insertPos + strlen(clipText));
                        if ((size_t)_caretPos >= _input.size()) _caretPos = -1;
                        publishText(oldText);
                        ensureCaretVisible();
                     }
                  }
                  return this;
               case InputInterface::KeyCode::KEY_RIGHT:
                  if (keyEvent.isDown && !_input.empty()) {
                     if (shiftHeld) {
                        // Start selection if not already selecting
                        if (!hasSelection()) setSelectionFromCaret();
                     }
                     if (_caretPos == -1) {
                        // Already at end, do nothing
                     } else {
                        _caretPos += 1;
                        if ((size_t)_caretPos >= _input.size()) _caretPos = -1;
                     }
                     if (shiftHeld) {
                        _selectionEnd = caretPosToIndex(_caretPos);
                     } else {
                        clearSelection();
                     }
                     ensureCaretVisible();
                  }
                  return this;
               case InputInterface::KeyCode::KEY_LEFT:
                  if (keyEvent.isDown && !_input.empty()) {
                     if (shiftHeld) {
                        if (!hasSelection()) setSelectionFromCaret();
                     }
                     if (_caretPos == -1) {
                        _caretPos = (int)_input.size() - 1;
                     } else if (_caretPos > 0) {
                        _caretPos -= 1;
                     }
                     if (shiftHeld) {
                        _selectionEnd = caretPosToIndex(_caretPos);
                     } else {
                        clearSelection();
                     }
                     ensureCaretVisible();
                  }
                  return this;
               case InputInterface::KeyCode::KEY_HOME:
                  if (keyEvent.isDown) {
                     if (shiftHeld) {
                        if (!hasSelection()) setSelectionFromCaret();
                     }
                     _caretPos = 0;
                     if (shiftHeld) {
                        _selectionEnd = 0;
                     } else {
                        clearSelection();
                     }
                     ensureCaretVisible();
                  }
                  return this;
               case InputInterface::KeyCode::KEY_END:
                  if (keyEvent.isDown) {
                     if (shiftHeld) {
                        if (!hasSelection()) setSelectionFromCaret();
                     }
                     _caretPos = -1;
                     if (shiftHeld) {
                        _selectionEnd = (int)_input.size();
                     } else {
                        clearSelection();
                     }
                     ensureCaretVisible();
                  }
                  return this;
               case InputInterface::KeyCode::KEY_BACKSPACE:
                  if (keyEvent.isDown) {
                     if (hasSelection()) {
                        deleteSelection();
                     } else if (!_input.empty()) {
                        // Backspace: delete character BEFORE caret
                        size_t caretIdx = (_caretPos == -1) ? _input.size() : (size_t)_caretPos;
                        if (caretIdx > 0) {
                           auto oldText = _input;
                           _input.erase(caretIdx - 1, 1);
                           _caretPos = (int)caretIdx - 1;
                           publishText(oldText);
                           ensureCaretVisible();
                        }
                     }
                     return this;
                  }
                  break;
               case InputInterface::KeyCode::KEY_DELETE:
                  if (keyEvent.isDown) {
                     if (hasSelection()) {
                        deleteSelection();
                     } else if (!_input.empty()) {
                        // Delete: delete character AT caret (after caret visually)
                        size_t caretIdx = (_caretPos == -1) ? _input.size() : (size_t)_caretPos;
                        if (caretIdx < _input.size()) {
                           auto oldText = _input;
                           _input.erase(caretIdx, 1);
                           // Caret position stays the same
                           publishText(oldText);
                           ensureCaretVisible();
                        }
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
   _isDragging = false;
   _scrollOffset = 0; // Reset scroll when not editing
   clearSelection();
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

///////////////////////////////////////////////////////////////////////////////////////
int LineEdit::caretPosToIndex(int pos) const {
   return pos == -1 ? (int)_input.size() : pos;
}

///////////////////////////////////////////////////////////////////////////////////////
int LineEdit::getSelectionMin() const {
   return std::min(_selectionStart, _selectionEnd);
}

///////////////////////////////////////////////////////////////////////////////////////
int LineEdit::getSelectionMax() const {
   return std::max(_selectionStart, _selectionEnd);
}

///////////////////////////////////////////////////////////////////////////////////////
std::string LineEdit::getSelectedText() const {
   if (!hasSelection()) return "";
   int minPos = getSelectionMin();
   int maxPos = getSelectionMax();
   return _input.substr(minPos, maxPos - minPos);
}

///////////////////////////////////////////////////////////////////////////////////////
void LineEdit::deleteSelection() {
   if (!hasSelection()) return;
   auto oldText = _input;
   int minPos = getSelectionMin();
   int maxPos = getSelectionMax();
   _input.erase(minPos, maxPos - minPos);
   _caretPos = minPos;
   clearSelection();
   publishText(oldText);
   ensureCaretVisible();
}

///////////////////////////////////////////////////////////////////////////////////////
void LineEdit::setSelectionFromCaret() {
   _selectionStart = _selectionEnd = caretPosToIndex(_caretPos);
}