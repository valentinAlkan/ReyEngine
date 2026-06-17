#include "RichTextEditor.h"
#include <algorithm>
#include <cstring>

using namespace std;
using namespace ReyEngine;

/////////////////////////////////////////////////////////////////////////////////////////
void RichTextEditor::_init() {
   setAcceptsHover(true);
}

/////////////////////////////////////////////////////////////////////////////////////////
float RichTextEditor::lineHeight() const {
   //use the font's reported height for a single glyph so empty lines still advance
   return measureText(" ", theme->font).y;
}

/////////////////////////////////////////////////////////////////////////////////////////
void RichTextEditor::setText(const std::string& text) {
   _assignString(text);
   if (_caret > text.size()) _caret = text.size();
}

/////////////////////////////////////////////////////////////////////////////////////////
void RichTextEditor::_assignString(const std::string& s) {
   auto oldText = getText().str();
   TrString next = getText(); //copy preserves language + key
   next.assign(s);
   TextRenderModel::setText(next);
   if (oldText != s) {
      EventTextChanged event(this);
      publish(event);
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
void RichTextEditor::caretRowCol(const std::string& text, size_t caret, size_t& row, size_t& col) const {
   if (caret > text.size()) caret = text.size();
   row = 0;
   size_t lastNewline = std::string::npos;
   for (size_t i = 0; i < caret; ++i) {
      if (text[i] == '\n') {
         ++row;
         lastNewline = i;
      }
   }
   col = (lastNewline == std::string::npos) ? caret : caret - (lastNewline + 1);
}

/////////////////////////////////////////////////////////////////////////////////////////
size_t RichTextEditor::lineStart(const std::string& text, size_t row) const {
   if (row == 0) return 0;
   size_t seen = 0;
   for (size_t i = 0; i < text.size(); ++i) {
      if (text[i] == '\n') {
         if (++seen == row) return i + 1;
      }
   }
   return text.size();
}

/////////////////////////////////////////////////////////////////////////////////////////
size_t RichTextEditor::caretFromMouse(const Pos<float>& localPos) const {
   const auto& text = getText().str();
   float lh = lineHeight();
   //which row was clicked
   size_t totalRows = (size_t)std::count(text.begin(), text.end(), '\n');
   int row = (int)((localPos.y - TEXT_MARGIN) / (lh > 0 ? lh : 1));
   if (row < 0) row = 0;
   if ((size_t)row > totalRows) row = (int)totalRows;

   size_t start = lineStart(text, (size_t)row);
   size_t end = text.find('\n', start);
   if (end == std::string::npos) end = text.size();
   std::string line = text.substr(start, end - start);

   float x = localPos.x - TEXT_MARGIN;
   if (x <= 0 || line.empty()) return start;
   if (x >= measureText(line, theme->font).x) return end; //past end of line
   if (auto idx = getCharIndexAt(line, {x, 0}, theme->font)) {
      return start + idx.value();
   }
   return end;
}

/////////////////////////////////////////////////////////////////////////////////////////
void RichTextEditor::moveCaretVertical(int dir) {
   const auto& text = getText().str();
   size_t row, col;
   caretRowCol(text, _caret, row, col);
   if (dir < 0 && row == 0) {_caret = 0; return;}
   size_t totalRows = (size_t)std::count(text.begin(), text.end(), '\n');
   if (dir > 0 && row >= totalRows) {_caret = text.size(); return;}

   size_t targetRow = (dir < 0) ? row - 1 : row + 1;
   size_t start = lineStart(text, targetRow);
   size_t end = text.find('\n', start);
   if (end == std::string::npos) end = text.size();
   size_t lineLen = end - start;
   _caret = start + std::min(col, lineLen); //keep the same column where possible
}

/////////////////////////////////////////////////////////////////////////////////////////
void RichTextEditor::render2D(RenderContext&) const {
   ScopeScissor scissor(getGlobalTransform(), getSizeRect().embiggen(1));
   const bool disabled = !getIsEnabled();
   drawRectangle(getSizeRect(), disabled ? theme->background.colorDisabled : theme->background.colorTertiary);

   const auto& text = getText().str();
   const auto& font = theme->font;
   const float lh = lineHeight();

   //draw each line
   float y = TEXT_MARGIN;
   size_t lineBegin = 0;
   while (true) {
      size_t nl = text.find('\n', lineBegin);
      size_t lineEnd = (nl == std::string::npos) ? text.size() : nl;
      std::string line = text.substr(lineBegin, lineEnd - lineBegin);
      if (!line.empty()) {
         drawText(line, {TEXT_MARGIN, y}, font, font->color, font->size, font->spacing);
      }
      y += lh;
      if (nl == std::string::npos) break;
      lineBegin = nl + 1;
   }

   //draw caret
   if (_isEditing) {
      auto frameCounter = getEngineFrameCount();
      if (frameCounter % 60 > 30) {
         size_t row, col;
         caretRowCol(text, _caret, row, col);
         size_t start = lineStart(text, row);
         std::string upToCaret = text.substr(start, _caret - start);
         float caretX = TEXT_MARGIN + measureText(upToCaret, font).x;
         float caretY = TEXT_MARGIN + row * lh;
         drawLine({{caretX, caretY}, {caretX, caretY + lh}}, 2, font->color);
      }
   }

   drawRectangleLines(getSizeRect(), 1.0, theme->background.colorPrimary);
}

/////////////////////////////////////////////////////////////////////////////////////////
void RichTextEditor::_on_focus_gained() {
   _isEditing = true;
}

/////////////////////////////////////////////////////////////////////////////////////////
void RichTextEditor::_on_focus_lost() {
   _isEditing = false;
}

/////////////////////////////////////////////////////////////////////////////////////////
Handled RichTextEditor::_unhandled_input(const InputEvent& event) {
   if (auto isMouse = event.isMouse()) {
      auto& mouse = isMouse.value();
      if (event.eventId == InputEventMouseButton::ID) {
         const auto& mouseEvent = event.toEvent<InputEventMouseButton>();
         if (getIsEnabled() && mouse->isInside()) {
            if (mouseEvent.isDown) {
               if (!isFocused()) setFocused(true);
               _caret = caretFromMouse(mouse->getLocalPos());
            }
            return this;
         } else if (isFocused() && !mouseEvent.isDown) {
            setFocused(false);
            return this;
         }
      }
   }

   if (!_isEditing || !isFocused()) return nullptr;

   switch (event.eventId) {
      case InputEventChar::ID: {
         const auto& charEvent = event.toEvent<InputEventChar>();
         auto text = getText().str();
         text.insert(_caret, 1, charEvent.ch);
         _caret += 1;
         _assignString(text);
         return this;
      }
      case InputEventKey::ID: {
         const auto& keyEvent = event.toEvent<InputEventKey>();
         if (!keyEvent.isDown) break;
         bool ctrlHeld = InputInterface::isKeyDown(InputInterface::KeyCode::KEY_LEFT_CONTROL) ||
                         InputInterface::isKeyDown(InputInterface::KeyCode::KEY_RIGHT_CONTROL);
         auto text = getText().str();
         switch (keyEvent.key) {
            default: break;
            case InputInterface::KeyCode::KEY_V:
               if (ctrlHeld) {
                  const char* clip = GetClipboardText();
                  if (clip && clip[0] != '\0') {
                     text.insert(_caret, clip);
                     _caret += strlen(clip);
                     _assignString(text);
                  }
                  return this;
               }
               break;
            case InputInterface::KeyCode::KEY_ENTER:
            case InputInterface::KeyCode::KEY_KP_ENTER:
               text.insert(_caret, 1, '\n');
               _caret += 1;
               _assignString(text);
               return this;
            case InputInterface::KeyCode::KEY_BACKSPACE:
               if (_caret > 0) {
                  text.erase(_caret - 1, 1);
                  _caret -= 1;
                  _assignString(text);
               }
               return this;
            case InputInterface::KeyCode::KEY_DELETE:
               if (_caret < text.size()) {
                  text.erase(_caret, 1);
                  _assignString(text);
               }
               return this;
            case InputInterface::KeyCode::KEY_LEFT:
               if (_caret > 0) _caret -= 1;
               return this;
            case InputInterface::KeyCode::KEY_RIGHT:
               if (_caret < text.size()) _caret += 1;
               return this;
            case InputInterface::KeyCode::KEY_UP:
               moveCaretVertical(-1);
               return this;
            case InputInterface::KeyCode::KEY_DOWN:
               moveCaretVertical(1);
               return this;
            case InputInterface::KeyCode::KEY_HOME: {
               size_t row, col;
               caretRowCol(text, _caret, row, col);
               _caret = lineStart(text, row);
               return this;
            }
            case InputInterface::KeyCode::KEY_END: {
               size_t nl = text.find('\n', _caret);
               _caret = (nl == std::string::npos) ? text.size() : nl;
               return this;
            }
         }
      }
   }
   return nullptr;
}
