#include "RichTextEditor.h"
#include <algorithm>
#include <cstring>

using namespace std;
using namespace ReyEngine;

/////////////////////////////////////////////////////////////////////////////////////////
void RichTextEditor::_init() {
   setAcceptsHover(true);
   //vertical scrollbar: a child Slider the tree renders/handles input for. It lives in a
   //right-edge gutter (positioned in updateScrollLayout). Dragging it sets the scroll offset.
   _vScrollBar = make_child<Slider>(getNode(), std::string("__rte_vbar"), Slider::SliderType::VERTICAL);
   _vScrollBar->setVisible(false); //hidden until updateScrollLayout decides content overflows
   subscribe<Slider::EventSliderValueChanged>(_vScrollBar, [this](const auto& e){
      _scroll.setOffsetY((float)e.value, /*fromBar*/true); //user drag -> offset, don't write back to the bar
   });
   _scroll.attachVBar(_vScrollBar);
}

/////////////////////////////////////////////////////////////////////////////////////////
void RichTextEditor::_process(float) {
   updateScrollLayout(); //content can grow/shrink every frame (typing); keep limits + bar current
}

/////////////////////////////////////////////////////////////////////////////////////////
float RichTextEditor::viewportWidth() const {
   //reserve the gutter only while the bar is actually shown (inset bar)
   return getWidth() - (_scroll.needsVBar() ? SCROLLBAR_WIDTH : 0.0f);
}

/////////////////////////////////////////////////////////////////////////////////////////
float RichTextEditor::contentHeight() const {
   return visualLines().size() * lineHeight() + 2 * TEXT_MARGIN;
}

/////////////////////////////////////////////////////////////////////////////////////////
void RichTextEditor::updateScrollLayout() {
   if (_vScrollBar) _vScrollBar->setRect({getWidth() - SCROLLBAR_WIDTH, 0, SCROLLBAR_WIDTH, getHeight()});
   _scroll.layout(contentHeight(), getHeight()); //clamps offset + syncs bar range/page/visibility
}

/////////////////////////////////////////////////////////////////////////////////////////
void RichTextEditor::ensureCaretVisible() {
   updateScrollLayout(); //limits must reflect the latest content before we test the caret row
   const auto& lines = visualLines();
   size_t row, col;
   rowColForCaret(lines, _caret, row, col);
   float top = TEXT_MARGIN + row * lineHeight();
   _scroll.ensureVisibleY(top, top + lineHeight());
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
      ++_textVersion; //invalidate the layout cache; the text content changed
      resetCaretBlink(); //any edit (incl. delete-forward) keeps the caret visible
      EventTextChanged event(this);
      publish(event);
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
float RichTextEditor::wrapWidth() const {
   //based on viewportWidth so wrapped text never runs under the inset scrollbar.
   //needsVBar() reflects last frame's layout, so bar appearance settles in one frame.
   return std::max(0.0f, viewportWidth() - 2 * TEXT_MARGIN);
}

/////////////////////////////////////////////////////////////////////////////////////////
std::vector<RichTextEditor::VisualLine> RichTextEditor::computeVisualLines(const std::string& text) const {
   std::vector<VisualLine> lines;
   const float maxW = wrapWidth();
   const auto& font = theme->font;
   size_t ls = 0;
   while (true) {
      size_t nl = text.find('\n', ls);
      size_t le = (nl == std::string::npos) ? text.size() : nl; //logical line is [ls, le)

      if (!_wordWrap || maxW <= 0) {
         lines.push_back({ls, le}); //no wrapping: one visual row per logical line
      } else {
         //Greedy wrap with an O(line) running width: each codepoint is measured once
         //and added to the segment width (plus inter-glyph spacing), instead of
         //re-measuring the whole segment every step.
         const float spacing = font->spacing;
         size_t segStart = ls;
         float segW = 0.0f;                       //width of the current row's text [segStart, i)
         bool firstGlyph = true;                  //no leading spacing before the first glyph
         size_t lastBreak = std::string::npos;    //byte just past the most recent space in this row
         size_t i = ls;
         while (i < le) {
            size_t next = nextCharBoundary(text, i);
            float glyphW = measureText(text.substr(i, next - i), font).x;
            float add = glyphW + (firstGlyph ? 0.0f : spacing);
            if (!firstGlyph && segW + add > maxW) {
               if (lastBreak != std::string::npos && lastBreak > segStart) {
                  lines.push_back({segStart, lastBreak}); //break after the last space that fit
                  segStart = lastBreak;
               } else {
                  lines.push_back({segStart, i});         //long word: break before this glyph
                  segStart = i;
               }
               i = segStart;                              //restart the new row at its first glyph
               segW = 0.0f; firstGlyph = true; lastBreak = std::string::npos;
               continue;
            }
            segW += add;                                  //accept the glyph onto this row
            firstGlyph = false;
            if (text[i] == ' ' || text[i] == '\t') lastBreak = next; //can break after this space
            i = next;
         }
         lines.push_back({segStart, le}); //the remainder of the logical line
      }

      if (nl == std::string::npos) break;
      ls = nl + 1; //skip the newline byte; next logical line starts after it
   }
   if (lines.empty()) lines.push_back({0, 0}); //empty text still has one (empty) row
   return lines;
}

/////////////////////////////////////////////////////////////////////////////////////////
const std::vector<RichTextEditor::VisualLine>& RichTextEditor::visualLines() const {
   const float w = wrapWidth();
   const float fontSize = theme->font->size;
   const float fontSpacing = theme->font->spacing;
   //rebuild only when something that affects layout has actually changed
   if (!_layoutValid || _layoutTextVersion != _textVersion || _layoutWidth != w ||
       _layoutWrap != _wordWrap || _layoutFontSize != fontSize || _layoutFontSpacing != fontSpacing) {
      _layoutCache = computeVisualLines(getText().str());
      _layoutValid = true;
      _layoutTextVersion = _textVersion;
      _layoutWidth = w;
      _layoutWrap = _wordWrap;
      _layoutFontSize = fontSize;
      _layoutFontSpacing = fontSpacing;
   }
   return _layoutCache;
}

/////////////////////////////////////////////////////////////////////////////////////////
void RichTextEditor::rowColForCaret(const std::vector<VisualLine>& lines, size_t caret, size_t& row, size_t& col) {
   row = 0;
   for (size_t i = 0; i < lines.size(); ++i) {
      if (lines[i].start <= caret) row = i; //largest row whose start is at/before the caret
      else break;
   }
   col = caret - lines[row].start;
}

/////////////////////////////////////////////////////////////////////////////////////////
void RichTextEditor::caretRowCol(const std::string& text, size_t caret, size_t& row, size_t& col) const {
   if (caret > text.size()) caret = text.size();
   rowColForCaret(computeVisualLines(text), caret, row, col);
}

/////////////////////////////////////////////////////////////////////////////////////////
size_t RichTextEditor::lineStart(const std::string& text, size_t row) const {
   auto lines = computeVisualLines(text);
   if (row >= lines.size()) return text.size();
   return lines[row].start;
}

/////////////////////////////////////////////////////////////////////////////////////////
size_t RichTextEditor::caretFromMouse(const Pos<float>& localPos) const {
   const auto& text = getText().str();
   const auto& lines = visualLines();
   float lh = lineHeight();
   //which visual row was clicked (translate the click into content space by the scroll offset)
   float contentY = localPos.y + _scroll.offsetY();
   int row = (int)((contentY - TEXT_MARGIN) / (lh > 0 ? lh : 1));
   if (row < 0) row = 0;
   if ((size_t)row >= lines.size()) row = (int)lines.size() - 1;

   const auto& vl = lines[(size_t)row];
   std::string line = text.substr(vl.start, vl.end - vl.start);

   float x = localPos.x - TEXT_MARGIN;
   if (x <= 0 || line.empty()) return vl.start;
   if (x >= measureText(line, theme->font).x) return vl.end; //past end of the visual row
   //getSubstrAt returns the text up to the clicked glyph; its byte length is the caret offset
   return vl.start + getSubstrAt(line, {x, 0}, theme->font).size();
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
   const auto& lines = visualLines();
   size_t row, col;
   rowColForCaret(lines, _caret, row, col);
   if (dir < 0 && row == 0) {_caret = 0; return;}
   if (dir > 0 && row + 1 >= lines.size()) {_caret = text.size(); return;}

   const auto& target = lines[(dir < 0) ? row - 1 : row + 1];
   size_t lineLen = target.end - target.start;
   _caret = target.start + std::min(col, lineLen); //keep the same column where possible
   //the byte column from the old row may land mid-codepoint here; snap back to a boundary
   if (_caret > target.start && _caret < target.end && (static_cast<unsigned char>(text[_caret]) & 0xC0) == 0x80) {
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
void RichTextEditor::drawSelection(const std::vector<VisualLine>& lines, float lh) const {
   if (!hasSelection()) return;
   const auto& text = getText().str();
   const auto& font = theme->font;
   size_t lo = selMin();
   size_t hi = selMax();

   size_t rowLo, colLo, rowHi, colHi;
   rowColForCaret(lines, lo, rowLo, colLo);
   rowColForCaret(lines, hi, rowHi, colHi);

   for (size_t row = rowLo; row <= rowHi; ++row) {
      const auto& vl = lines[row];
      std::string line = text.substr(vl.start, vl.end - vl.start);

      //columns of the selection on this row, clamped to the visual line
      size_t cStart = std::min((row == rowLo) ? colLo : (size_t)0, line.size());
      size_t cEnd   = std::min((row == rowHi) ? colHi : line.size(), line.size());
      float x0 = TEXT_MARGIN + measureText(line.substr(0, cStart), font).x;
      float x1 = TEXT_MARGIN + measureText(line.substr(0, cEnd), font).x;
      //rows the selection continues past extend a little to signal the wrap/newline
      if (row != rowHi) x1 += measureText(" ", font).x;
      float y = TEXT_MARGIN + row * lh - _scroll.offsetY(); //shift by the scroll offset
      drawRectangle({{x0, y}, {x1 - x0, lh}}, theme->foreground.colorHighlight);
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
void RichTextEditor::render2D(RenderContext&) const {
   const bool disabled = !getIsEnabled();
   drawRectangle(getSizeRect(), disabled ? theme->background.colorDisabled : theme->background.colorTertiary);

   const auto& text = getText().str();
   const auto& font = theme->font;
   const float lh = lineHeight();
   const auto& lines = visualLines(); //cached layout; rebuilt only when text/width/wrap/font change
   const float offsetY = _scroll.offsetY();

   //clip text/selection/caret to the area left of the scrollbar gutter (inset when the bar shows)
   {
      ScopeScissor scissor(getGlobalTransform(), Rect<float>(0, 0, viewportWidth(), getHeight()));

      //draw selection highlight beneath the text
      drawSelection(lines, lh);

      //draw only the visual rows that intersect the viewport (virtualized)
      if (lh > 0) {
         size_t firstRow = (offsetY > TEXT_MARGIN) ? (size_t)((offsetY - TEXT_MARGIN) / lh) : 0;
         for (size_t row = firstRow; row < lines.size(); ++row) {
            float y = TEXT_MARGIN + row * lh - offsetY;
            if (y >= getHeight()) break; //first row past the bottom edge: the rest are too
            const auto& vl = lines[row];
            if (vl.end > vl.start) {
               std::string line = text.substr(vl.start, vl.end - vl.start);
               drawText(line, {TEXT_MARGIN, y}, font, font->color, font->size, font->spacing);
            }
         }
      }

      //draw caret - blink relative to _caretBlinkBase so a fresh move shows the caret right away
      if (_isEditing) {
         auto elapsed = getEngineFrameCount() - _caretBlinkBase;
         if (elapsed % 60 < 30) {
            size_t row, col;
            rowColForCaret(lines, _caret, row, col);
            std::string upToCaret = text.substr(lines[row].start, col);
            float caretX = TEXT_MARGIN + measureText(upToCaret, font).x;
            float caretY = TEXT_MARGIN + row * lh - offsetY;
            drawLine({{caretX, caretY}, {caretX, caretY + lh}}, 2, font->color);
         }
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
   if (_caret != caretBefore || _didEdit) ensureCaretVisible(); //keep the caret on-screen after edits/moves
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
         case InputEventMouseWheel::ID: {
            if (mouse->isInside() && _scroll.needsVBar()) {
               constexpr float WHEEL_SPEED = 30.0f; //pixels per wheel notch
               const auto& wheelEvent = event.toEvent<InputEventMouseWheel>();
               _scroll.scrollByY(-wheelEvent.wheelMove.y * WHEEL_SPEED);
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
               const auto& lines = visualLines();
               size_t row, col;
               rowColForCaret(lines, _caret, row, col);
               size_t start = lines[row].start;
               size_t end = lines[row].end; //bound to the visual row, not the logical line
               //first non-whitespace char on the visual row
               size_t firstNonWs = start;
               while (firstNonWs < end && isWordSpace(text[firstNonWs])) ++firstNonWs;
               //already at column 0 with leading whitespace -> jump to first non-whitespace,
               //otherwise go to the start of the row
               if (_caret == start && firstNonWs > start && firstNonWs < end) {
                  _caret = firstNonWs;
               } else {
                  _caret = start;
               }
               if (!shiftHeld) clearSelection();
               return this;
            }
            case InputInterface::KeyCode::KEY_END: {
               const auto& text = getText().str();
               const auto& lines = visualLines();
               size_t row, col;
               rowColForCaret(lines, _caret, row, col);
               _caret = lines[row].end; //end of the visual row
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