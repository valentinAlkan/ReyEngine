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
   auto normalized = normalizeNewlines(text);
   _assignString(normalized);
   if (_caret > normalized.size()) _caret = normalized.size();
   clearSelection(); //drop any stale selection that could point past the new text
}

/////////////////////////////////////////////////////////////////////////////////////////
void RichTextEditor::_assignString(const std::string& s) {
   auto oldText = getText().str();
   TrString next = getText(); //copy preserves language + key
   next.assign(s);
   TextRenderModel::setText(next);
   if (oldText != s) {
      resetCaretBlink(); //any edit (incl. delete-forward) keeps the caret visible
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
   //getSubstrAt returns the text up to the clicked glyph; its byte length is the caret offset
   return start + getSubstrAt(line, {x, 0}, theme->font).size();
}

/////////////////////////////////////////////////////////////////////////////////////////
size_t RichTextEditor::nextCharBoundary(const std::string& text, size_t i) {
   if (i >= text.size()) return text.size();
   ++i; //skip the lead byte, then any UTF-8 continuation bytes (10xxxxxx)
   while (i < text.size() && (static_cast<unsigned char>(text[i]) & 0xC0) == 0x80) ++i;
   return i;
}

/////////////////////////////////////////////////////////////////////////////////////////
size_t RichTextEditor::prevCharBoundary(const std::string& text, size_t i) {
   if (i == 0) return 0;
   --i; //step back over the trailing continuation bytes to the lead byte
   while (i > 0 && (static_cast<unsigned char>(text[i]) & 0xC0) == 0x80) --i;
   return i;
}

/////////////////////////////////////////////////////////////////////////////////////////
std::string RichTextEditor::normalizeNewlines(std::string text) {
   std::string out;
   out.reserve(text.size());
   for (size_t i = 0; i < text.size(); ++i) {
      if (text[i] == '\r') {
         out.push_back('\n');
         if (i + 1 < text.size() && text[i + 1] == '\n') ++i; //collapse a \r\n pair
      } else {
         out.push_back(text[i]);
      }
   }
   return out;
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
   //the byte column from the old line may land mid-codepoint here; snap back to a boundary
   if (_caret > start && _caret < end && (static_cast<unsigned char>(text[_caret]) & 0xC0) == 0x80) {
      _caret = prevCharBoundary(text, _caret);
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
std::string RichTextEditor::getSelectedText() const {
   if (!hasSelection()) return "";
   return getText().str().substr(selMin(), selMax() - selMin());
}

/////////////////////////////////////////////////////////////////////////////////////////
void RichTextEditor::deleteSelection() {
   if (!hasSelection()) return;
   auto text = getText().str();
   size_t lo = selMin();
   text.erase(lo, selMax() - lo);
   _caret = lo;
   clearSelection();
   _assignString(text);
}

/////////////////////////////////////////////////////////////////////////////////////////
void RichTextEditor::drawSelection(float lh) const {
   if (!hasSelection()) return;
   const auto& text = getText().str();
   const auto& font = theme->font;
   size_t lo = selMin();
   size_t hi = selMax();

   size_t rowLo, colLo, rowHi, colHi;
   caretRowCol(text, lo, rowLo, colLo);
   caretRowCol(text, hi, rowHi, colHi);

   for (size_t row = rowLo; row <= rowHi; ++row) {
      size_t start = lineStart(text, row);
      size_t end = text.find('\n', start);
      if (end == std::string::npos) end = text.size();
      std::string line = text.substr(start, end - start);

      //columns of the selection on this row, clamped to the line
      size_t cStart = (row == rowLo) ? colLo : 0;
      size_t cEnd   = (row == rowHi) ? colHi : line.size();
      float x0 = TEXT_MARGIN + measureText(line.substr(0, cStart), font).x;
      float x1 = TEXT_MARGIN + measureText(line.substr(0, cEnd), font).x;
      //rows fully inside the selection extend a little past EOL to signal the newline
      if (row != rowHi) x1 += measureText(" ", font).x;
      float y = TEXT_MARGIN + row * lh;
      drawRectangle({{x0, y}, {x1 - x0, lh}}, theme->foreground.colorHighlight);
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
void RichTextEditor::render2D(RenderContext&) const {
   ScopeScissor scissor(getGlobalTransform(), getSizeRect().embiggen(1));
   const bool disabled = !getIsEnabled();
   drawRectangle(getSizeRect(), disabled ? theme->background.colorDisabled : theme->background.colorTertiary);

   const auto& text = getText().str();
   const auto& font = theme->font;
   const float lh = lineHeight();

   //draw selection highlight beneath the text
   drawSelection(lh);

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

   //draw caret - blink relative to _caretBlinkBase so a fresh move shows the caret right away
   if (_isEditing) {
      auto elapsed = getEngineFrameCount() - _caretBlinkBase;
      if (elapsed % 60 < 30) {
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
   resetCaretBlink(); //caret visible the moment editing starts
}

/////////////////////////////////////////////////////////////////////////////////////////
void RichTextEditor::_on_focus_lost() {
   _isEditing = false;
}

/////////////////////////////////////////////////////////////////////////////////////////
Handled RichTextEditor::_unhandled_input(const InputEvent& event) {
   size_t caretBefore = _caret;
   Handled result = _processEdit(event);
   if (_caret != caretBefore) resetCaretBlink(); //caret moved -> show it immediately
   return result;
}

/////////////////////////////////////////////////////////////////////////////////////////
Handled RichTextEditor::_processEdit(const InputEvent& event) {
   if (auto isMouse = event.isMouse()) {
      auto& mouse = isMouse.value();
      switch (event.eventId) {
         case InputEventMouseButton::ID: {
            const auto& mouseEvent = event.toEvent<InputEventMouseButton>();
            if (getIsEnabled() && mouse->isInside()) {
               if (mouseEvent.isDown) {
                  if (!isFocused()) setFocused(true);
                  _caret = caretFromMouse(mouse->getLocalPos());
                  clearSelection();      //new click starts a fresh selection at the caret
                  _isDragging = true;
               } else {
                  _isDragging = false;   //button released
               }
               return this;
            } else if (isFocused() && !mouseEvent.isDown) {
               setFocused(false);
               _isDragging = false;
               return this;
            }
            break;
         }
         case InputEventMouseMotion::ID: {
            if (_isDragging && isFocused()) {
               //extend selection: move the caret, leave the anchor put
               _caret = caretFromMouse(mouse->getLocalPos());
               return this;
            }
            break;
         }
      }
   }

   if (!_isEditing || !isFocused()) return nullptr;

   switch (event.eventId) {
      case InputEventChar::ID: {
         const auto& charEvent = event.toEvent<InputEventChar>();
         //drop control characters (incl. \r and \n); newlines are inserted via the Enter key,
         //and other control codepoints have no glyph and would render as '?'
         if (charEvent.ch < 0x20 || charEvent.ch == 0x7F) break;
         //charEvent.ch is a Unicode codepoint; encode it to UTF-8 before storing
         int byteCount = 0;
         const char* utf8 = CodepointToUTF8(charEvent.ch, &byteCount);
         if (byteCount <= 0) return this;
         deleteSelection();                 //typing over a selection replaces it
         auto text = getText().str();
         text.insert(_caret, utf8, byteCount);
         _caret += byteCount;
         clearSelection();
         _assignString(text);
         return this;
      }
      case InputEventKey::ID: {
         const auto& keyEvent = event.toEvent<InputEventKey>();
         if (!keyEvent.isDown) break;
         bool ctrlHeld = InputInterface::isKeyDown(InputInterface::KeyCode::KEY_LEFT_CONTROL) ||
                         InputInterface::isKeyDown(InputInterface::KeyCode::KEY_RIGHT_CONTROL);
         bool shiftHeld = InputInterface::isKeyDown(InputInterface::KeyCode::KEY_LEFT_SHIFT) ||
                          InputInterface::isKeyDown(InputInterface::KeyCode::KEY_RIGHT_SHIFT);
         switch (keyEvent.key) {
            default: break;
            case InputInterface::KeyCode::KEY_A:
               if (ctrlHeld) { //select all
                  _selectionAnchor = 0;
                  _caret = getText().str().size();
               }
               return this;
            case InputInterface::KeyCode::KEY_C:
               if (ctrlHeld && hasSelection()) SetClipboardText(getSelectedText().c_str());
               return this;
            case InputInterface::KeyCode::KEY_X:
               if (ctrlHeld && hasSelection()) {
                  SetClipboardText(getSelectedText().c_str());
                  deleteSelection();
               }
               return this;
            case InputInterface::KeyCode::KEY_V:
               if (ctrlHeld) {
                  const char* clip = GetClipboardText();
                  if (clip && clip[0] != '\0') {
                     std::string pasted = normalizeNewlines(clip); //strip \r so it won't render as '?'
                     deleteSelection();
                     auto text = getText().str();
                     text.insert(_caret, pasted);
                     _caret += pasted.size();
                     clearSelection();
                     _assignString(text);
                  }
                  return this;
               }
               break;
            case InputInterface::KeyCode::KEY_ENTER:
            case InputInterface::KeyCode::KEY_KP_ENTER: {
               deleteSelection();
               auto text = getText().str();
               text.insert(_caret, 1, '\n');
               _caret += 1;
               clearSelection();
               _assignString(text);
               return this;
            }
            case InputInterface::KeyCode::KEY_BACKSPACE:
               if (hasSelection()) {
                  deleteSelection();
               } else if (_caret > 0) {
                  auto text = getText().str();
                  size_t prev = prevCharBoundary(text, _caret);
                  text.erase(prev, _caret - prev); //erase the whole codepoint
                  _caret = prev;
                  clearSelection();
                  _assignString(text);
               }
               return this;
            case InputInterface::KeyCode::KEY_DELETE:
               if (hasSelection()) {
                  deleteSelection();
               } else if (_caret < getText().str().size()) {
                  auto text = getText().str();
                  size_t next = nextCharBoundary(text, _caret);
                  text.erase(_caret, next - _caret); //erase the whole codepoint
                  _assignString(text);
               }
               return this;
            case InputInterface::KeyCode::KEY_LEFT:
               if (!shiftHeld && hasSelection()) {
                  _caret = selMin();            //collapse selection to its left edge
               } else if (_caret > 0) {
                  _caret = prevCharBoundary(getText().str(), _caret);
               }
               if (!shiftHeld) clearSelection();
               return this;
            case InputInterface::KeyCode::KEY_RIGHT:
               if (!shiftHeld && hasSelection()) {
                  _caret = selMax();            //collapse selection to its right edge
               } else if (_caret < getText().str().size()) {
                  _caret = nextCharBoundary(getText().str(), _caret);
               }
               if (!shiftHeld) clearSelection();
               return this;
            case InputInterface::KeyCode::KEY_UP:
               moveCaretVertical(-1);
               if (!shiftHeld) clearSelection();
               return this;
            case InputInterface::KeyCode::KEY_DOWN:
               moveCaretVertical(1);
               if (!shiftHeld) clearSelection();
               return this;
            case InputInterface::KeyCode::KEY_HOME: {
               const auto& text = getText().str();
               size_t row, col;
               caretRowCol(text, _caret, row, col);
               _caret = lineStart(text, row);
               if (!shiftHeld) clearSelection();
               return this;
            }
            case InputInterface::KeyCode::KEY_END: {
               size_t nl = getText().str().find('\n', _caret);
               _caret = (nl == std::string::npos) ? getText().str().size() : nl;
               if (!shiftHeld) clearSelection();
               return this;
            }
         }
      }
   }
   return nullptr;
}
