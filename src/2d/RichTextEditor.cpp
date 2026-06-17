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
   resetHistory();   //wholesale replacement isn't an undoable edit; old actions' offsets are now invalid
}

/////////////////////////////////////////////////////////////////////////////////////////
void RichTextEditor::pushAction(std::shared_ptr<EditAction> action, bool mergeable) {
   action->redo(*this);  //apply the edit immediately
   //a brand-new edit invalidates any redo branch hanging off the cursor
   if (_undoCursor < _undo.size()) _undo.erase(_undo.begin() + (long)_undoCursor, _undo.end());
   //fold continuous typing/deleting into the previous step when allowed
   if (mergeable && _coalesce && !_undo.empty() && _undo.back()->tryMerge(*action)) {
      //absorbed into _undo.back(); nothing appended, cursor already == size
   } else {
      _undo.push_back(std::move(action));
      _undoCursor = _undo.size();
   }
   _coalesce = mergeable; //only a mergeable edit lets the *next* one coalesce into it
   _didEdit = true;       //tell _unhandled_input this caret move came from an edit, not navigation
}

/////////////////////////////////////////////////////////////////////////////////////////
void RichTextEditor::undo() {
   if (_undoCursor == 0) return;
   _undo[--_undoCursor]->undo(*this);
   breakUndoMerge(); //an undo ends any in-progress typing run
}

/////////////////////////////////////////////////////////////////////////////////////////
void RichTextEditor::redo() {
   if (_undoCursor >= _undo.size()) return;
   _undo[_undoCursor++]->redo(*this);
   breakUndoMerge();
}

/////////////////////////////////////////////////////////////////////////////////////////
void RichTextEditor::replaceSelectionWith(const TrString& text, bool mergeable) {
   if (hasSelection()) {
      //overwrite the highlight as one undo step: remove the selection, then insert
      auto comp = std::make_shared<CompositeAction>();
      comp->actions.push_back(std::make_shared<RemoveTextAction>(selMin(), TrString(getSelectedText())));
      comp->actions.push_back(std::make_shared<InsertTextAction>(selMin(), text));
      pushAction(comp, false); //a replace is always a discrete step, never coalesced
   } else {
      pushAction(std::make_shared<InsertTextAction>(_caret, text), mergeable);
   }
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
static bool isWordSpace(char c) {
   return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

/////////////////////////////////////////////////////////////////////////////////////////
size_t RichTextEditor::nextWordBoundary(const std::string& text, size_t i) {
   while (i < text.size() && isWordSpace(text[i])) ++i;  //skip whitespace
   while (i < text.size() && !isWordSpace(text[i])) ++i; //skip the word
   return i;
}

/////////////////////////////////////////////////////////////////////////////////////////
size_t RichTextEditor::prevWordBoundary(const std::string& text, size_t i) {
   while (i > 0 && isWordSpace(text[i - 1])) --i;  //skip whitespace
   while (i > 0 && !isWordSpace(text[i - 1])) --i; //skip the word
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
   //a selection delete is its own discrete undo step; RemoveTextAction::redo
   //erases the run, collapses the caret to selMin and clears the selection
   pushAction(std::make_shared<RemoveTextAction>(selMin(), TrString(getSelectedText())), false);
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
   _didEdit = false;
   Handled result = _processEdit(event);
   if (_caret != caretBefore) {
      resetCaretBlink();                   //caret moved -> show it immediately
      if (!_didEdit) breakUndoMerge();     //moved by navigation, not an edit -> end the typing run
   }
   return result;
}

/////////////////////////////////////////////////////////////////////////////////////////
Handled RichTextEditor::_processEdit(const InputEvent& event) {
   if (auto isMouse = event.isMouse()) {
      auto& mouse = isMouse.value();
      switch (event.eventId) {
         case InputEventMouseButton::ID: {
            const auto& mouseEvent = event.toEvent<InputEventMouseButton>();
            if (mouseEvent.isDown) {
               if (getIsEnabled() && mouse->isInside()) {
                  if (!isFocused()) setFocused(true);
                  _caret = caretFromMouse(mouse->getLocalPos());
                  clearSelection();      //new click starts a fresh selection at the caret
                  _isDragging = true;
                  return this;
               } else if (isFocused()) {
                  setFocused(false);     //pressed elsewhere -> stop editing
                  _isDragging = false;
               }
               break; //not consuming: let the click reach whatever was pressed
            }
            //button released: end the drag but keep focus, even if released outside the editor
            //(otherwise dragging a selection off the widget would defocus and break editing)
            if (_isDragging) {
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
         //typing replaces any selection then inserts; plain typing is mergeable so
         //a run of keystrokes collapses into a single undo step
         replaceSelectionWith(TrString(std::string(utf8, (size_t)byteCount)), /*mergeable*/true);
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
            case InputInterface::KeyCode::KEY_Z:
               if (ctrlHeld) { if (shiftHeld) redo(); else undo(); } //Ctrl+Z undo, Ctrl+Shift+Z redo
               return this;
            case InputInterface::KeyCode::KEY_Y:
               if (ctrlHeld) redo(); //Ctrl+Y redo (Windows-style)
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
                     replaceSelectionWith(TrString(pasted), /*mergeable*/false); //paste is one discrete step
                  }
                  return this;
               }
               break;
            case InputInterface::KeyCode::KEY_ENTER:
            case InputInterface::KeyCode::KEY_KP_ENTER:
               replaceSelectionWith(TrString("\n"), /*mergeable*/false); //a newline is an undo boundary
               return this;
            case InputInterface::KeyCode::KEY_BACKSPACE:
               if (hasSelection()) {
                  deleteSelection();
               } else if (_caret > 0) {
                  const auto& text = getText().str();
                  size_t prev = prevCharBoundary(text, _caret); //erase the whole codepoint
                  pushAction(std::make_shared<RemoveTextAction>(prev, TrString(text.substr(prev, _caret - prev))), /*mergeable*/true);
               }
               return this;
            case InputInterface::KeyCode::KEY_DELETE:
               if (hasSelection()) {
                  deleteSelection();
               } else if (_caret < getText().str().size()) {
                  const auto& text = getText().str();
                  size_t next = nextCharBoundary(text, _caret); //erase the whole codepoint
                  pushAction(std::make_shared<RemoveTextAction>(_caret, TrString(text.substr(_caret, next - _caret))), /*mergeable*/true);
               }
               return this;
            case InputInterface::KeyCode::KEY_LEFT:
               if (ctrlHeld) {
                  _caret = prevWordBoundary(getText().str(), _caret); //jump a word
               } else if (!shiftHeld && hasSelection()) {
                  _caret = selMin();            //collapse selection to its left edge
               } else if (_caret > 0) {
                  _caret = prevCharBoundary(getText().str(), _caret);
               }
               if (!shiftHeld) clearSelection();
               return this;
            case InputInterface::KeyCode::KEY_RIGHT:
               if (ctrlHeld) {
                  _caret = nextWordBoundary(getText().str(), _caret); //jump a word
               } else if (!shiftHeld && hasSelection()) {
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
               size_t start = lineStart(text, row);
               size_t end = text.find('\n', start);
               if (end == std::string::npos) end = text.size();
               //first non-whitespace char on the line
               size_t firstNonWs = start;
               while (firstNonWs < end && isWordSpace(text[firstNonWs])) ++firstNonWs;
               //already at column 0 with leading whitespace -> jump to first non-whitespace,
               //otherwise go to the start of the line
               if (_caret == start && firstNonWs > start && firstNonWs < end) {
                  _caret = firstNonWs;
               } else {
                  _caret = start;
               }
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

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
void RichTextEditor::InsertTextAction::redo(RichTextEditor& e) {
   auto s = e.getText().str();
   s.insert(index, text);              //put the run back at the recorded offset
   e._caret = index + text.size();     //caret lands just after the inserted run
   e.clearSelection();
   e._assignString(s);                 //write back + fire EventTextChanged
}

/////////////////////////////////////////////////////////////////////////////////////////
void RichTextEditor::InsertTextAction::undo(RichTextEditor& e) {
   auto s = e.getText().str();
   s.erase(index, text.size());        //remove exactly what redo() inserted
   e._caret = index;                   //caret returns to where the run started
   e.clearSelection();
   e._assignString(s);
}

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
void RichTextEditor::RemoveTextAction::redo(RichTextEditor& e) {
   auto s = e.getText().str();
   s.erase(index, text.size());        //delete the recorded run
   e._caret = index;                   //caret collapses to the deletion point
   e.clearSelection();
   e._assignString(s);                 //write back + fire EventTextChanged
}

/////////////////////////////////////////////////////////////////////////////////////////
void RichTextEditor::RemoveTextAction::undo(RichTextEditor& e) {
   auto s = e.getText().str();
   s.insert(index, text);              //restore exactly what redo() removed
   e._caret = index + text.size();     //caret lands just after the restored run
   e.clearSelection();
   e._assignString(s);
}

/////////////////////////////////////////////////////////////////////////////////////////
bool RichTextEditor::InsertTextAction::tryMerge(const EditAction& next) {
   auto* o = dynamic_cast<const InsertTextAction*>(&next);
   if (!o) return false;                              //only merge insert-into-insert
   if (o->index != index + text.size()) return false; //must continue right where we left off
   //stop a merge at line boundaries so undo lands sensibly per line
   if (!text.empty() && text.back() == '\n') return false;
   if (!o->text.empty() && o->text.front() == '\n') return false;
   text += o->text;                                   //absorb the newer run
   return true;
}

/////////////////////////////////////////////////////////////////////////////////////////
bool RichTextEditor::RemoveTextAction::tryMerge(const EditAction& next) {
   auto* o = dynamic_cast<const RemoveTextAction*>(&next);
   if (!o) return false;                              //only merge remove-into-remove
   //forward-delete: each press removes at the same offset, so runs are consecutive
   if (o->index == index) {
      if (!text.empty() && text.back() == '\n') return false;
      if (!o->text.empty() && o->text.front() == '\n') return false;
      text += o->text;
      return true;
   }
   //backspace: each press removes just before the previous one, ending where we began
   if (o->index + o->text.size() == index) {
      if (!o->text.empty() && o->text.back() == '\n') return false;
      if (!text.empty() && text.front() == '\n') return false;
      text = o->text + text;                          //prepend the earlier-in-buffer run
      index = o->index;                               //deletion now starts further left
      return true;
   }
   return false;
}

/////////////////////////////////////////////////////////////////////////////////////////
void RichTextEditor::CompositeAction::redo(RichTextEditor& e) {
   for (auto& a : actions) a->redo(e);                //replay front-to-back
}

/////////////////////////////////////////////////////////////////////////////////////////
void RichTextEditor::CompositeAction::undo(RichTextEditor& e) {
   for (auto it = actions.rbegin(); it != actions.rend(); ++it) (*it)->undo(e); //reverse order
}