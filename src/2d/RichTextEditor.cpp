#include "RichTextEditor.h"
#include <algorithm>

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

   //right-click context menu: a hidden DropDownMenu child, rebuilt each time it opens.
   //Selecting an entry runs the matching clipboard/selection op.
   _contextMenu = make_child<DropDownMenu>(getNode(), std::string("__rte_ctxmenu"));
   _contextMenu->setVisible(false);
   subscribe<DropDownMenu::EventItemSelected>(_contextMenu, [this](const auto& e){
      const auto& label = e.item->getText();
      if (label == "Copy") _edit.copySelection();
      else if (label == "Paste") _edit.pasteClipboard();
      else if (label == "Select All") _edit.selectAll();
   });
   //when the menu closes, hand focus back to the editor if the dismissing click landed
   //inside it (picked an entry, or clicked back into the text), so editing resumes
   subscribe<DropDownMenu::EventAboutToHide>(_contextMenu, [this](const auto&){
      if (getSizeRect().contains(getLocalMousePos())) setFocused(true);
   });
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
   rowColForCaret(lines, _edit.caret(), row, col);
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
   auto normalized = TextEditHandler::normalizeNewlines(text);
   _assignString(normalized);
   _edit.textReset();    //clamp caret, drop selection + history (old offsets are invalid now)
   updateScrollLayout(); //new content height -> re-clamp scroll limits + refresh the bar now
}

/////////////////////////////////////////////////////////////////////////////////////////
void RichTextEditor::_assignString(const std::string& s) {
   if (getText().str() == s) return; //no change: skip the write and the notifications
   TrString next = getText(); //copy preserves language + key
   next.assign(s);
   //write through the shared model: this publishes EventTextChanged, which calls
   //_on_text_changed() on THIS editor and on every other view aliasing the model
   //(invalidating their caches + re-laying-out), so they all stay in sync.
   getModel()->setText(next);
   EventTextChanged event(this); //editor-level event for external listeners (distinct from the model's)
   publish(event);
}

/////////////////////////////////////////////////////////////////////////////////////////
void RichTextEditor::_on_text_changed() {
   ++_textVersion; //the shared text changed (here or via another view): drop the layout cache
   //a remote edit can shrink the buffer out from under the caret/selection; clamp them back in
   _edit.externalTextChanged();
   updateScrollLayout(); //new content height -> re-clamp scroll limits + refresh the bar
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
            size_t next = TextEditHandler::nextCharBoundary(text, i);
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
size_t RichTextEditor::caretVertical(size_t caret, int dir) const {
   const auto& text = getText().str();
   const auto& lines = visualLines();
   size_t row, col;
   rowColForCaret(lines, caret, row, col);
   if (dir < 0 && row == 0) return 0;
   if (dir > 0 && row + 1 >= lines.size()) return text.size();

   const auto& target = lines[(dir < 0) ? row - 1 : row + 1];
   size_t lineLen = target.end - target.start;
   size_t c = target.start + std::min(col, lineLen); //keep the same column where possible
   //the byte column from the old row may land mid-codepoint here; snap back to a boundary
   if (c > target.start && c < target.end && (static_cast<unsigned char>(text[c]) & 0xC0) == 0x80) {
      c = TextEditHandler::prevCharBoundary(text, c);
   }
   return c;
}

/////////////////////////////////////////////////////////////////////////////////////////
size_t RichTextEditor::rowStart(size_t caret) const {
   const auto& lines = visualLines();
   size_t row, col;
   rowColForCaret(lines, caret, row, col);
   return lines[row].start;
}

/////////////////////////////////////////////////////////////////////////////////////////
size_t RichTextEditor::rowEnd(size_t caret) const {
   const auto& lines = visualLines();
   size_t row, col;
   rowColForCaret(lines, caret, row, col);
   return lines[row].end;
}

/////////////////////////////////////////////////////////////////////////////////////////
void RichTextEditor::openContextMenu(const Pos<float>& localPos) {
   if (!_contextMenu) return;
   //rebuild entries each open so Copy/Paste only show when there's something to act on
   _contextMenu->clear();
   if (_edit.hasSelection()) _contextMenu->addEntry("Copy");
   const char* clip = GetClipboardText();
   if (clip && clip[0] != '\0') _contextMenu->addEntry("Paste");
   _contextMenu->addEntry("Select All");
   _contextMenu->setPosition(localPos);
   _contextMenu->open();
   _contextMenu->setFocused(true); //take focus so the menu owns input until dismissed
}

/////////////////////////////////////////////////////////////////////////////////////////
void RichTextEditor::drawSelection(const std::vector<VisualLine>& lines, float lh) const {
   if (!_edit.hasSelection()) return;
   const auto& text = getText().str();
   const auto& font = theme->font;
   size_t lo = _edit.selMin();
   size_t hi = _edit.selMax();

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

      //draw caret - the handler owns the blink phase and resets it on caret moves/edits
      if (_edit.isEditing() && _edit.caretVisible()) {
         size_t row, col;
         rowColForCaret(lines, _edit.caret(), row, col);
         std::string upToCaret = text.substr(lines[row].start, col);
         float caretX = TEXT_MARGIN + measureText(upToCaret, font).x;
         float caretY = TEXT_MARGIN + row * lh - offsetY;
         drawLine({{caretX, caretY}, {caretX, caretY + lh}}, 2, font->color);
      }
   }

   drawRectangleLines(getSizeRect(), 1.0, theme->background.colorPrimary);
}

/////////////////////////////////////////////////////////////////////////////////////////
void RichTextEditor::_on_focus_gained() {
   _edit.setEditing(true); //starts editing + resets the blink so the caret shows immediately
}

/////////////////////////////////////////////////////////////////////////////////////////
void RichTextEditor::_on_focus_lost() {
   _edit.setEditing(false);
}

/////////////////////////////////////////////////////////////////////////////////////////
void RichTextEditor::_on_rect_changed() {
   //a resize changes both the gutter's x (getWidth) and the usable height: move the bar
   //and re-clamp the scroll limits now, instead of waiting for the next edit/caret move.
   updateScrollLayout();
}

/////////////////////////////////////////////////////////////////////////////////////////
Handled RichTextEditor::_unhandled_input(const InputEvent& event) {
   //wheel scrolling is view-level, not an edit: handle it here before delegating
   if (auto isMouse = event.isMouse()) {
      if (event.eventId == InputEventMouseWheel::ID && isMouse.value()->isInside() && _scroll.needsVBar()) {
         constexpr float WHEEL_SPEED = 30.0f; //pixels per wheel notch
         const auto& wheelEvent = event.toEvent<InputEventMouseWheel>();
         _scroll.scrollByY(-wheelEvent.wheelMove.y * WHEEL_SPEED);
         return this;
      }
   }
   //everything else (caret/selection/keyboard editing) is the handler's job
   if (_edit.handleInput(event)) return this;
   return nullptr;
}
