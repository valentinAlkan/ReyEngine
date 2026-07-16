#include "LineEdit.h"

using namespace std;
using namespace ReyEngine;

///////////////////////////////////////////////////////////////////////////////////////
void LineEdit::render2D(RenderContext&) const {
   ScopeScissor scissor(getGlobalTransform(), getSizeRect().embiggen(1));
   bool disabled = !getIsEnabled();
   drawRectangle(getSizeRect(), disabled ? theme->background.colorDisabled : theme->background.colorTertiary);

   auto& font = theme->font;
   const auto& input = getModel()->getText().str();

   //draw default text
   bool isDefaultText = input.empty();
   const auto& displayText = isDefaultText ? _defaultText : input;
   auto textSize = measureText(displayText, font);
   float textStartX = TEXT_MARGIN - _scrollOffset;
   float textPosV = (getHeight() - textSize.y) / 2;

   if (!displayText.empty() && (!isDefaultText || !_edit.isEditing())) {
      // Draw selection highlight
      if (_edit.hasSelection() && !input.empty()) {
         float selStartX = textStartX + measureText(input.substr(0, _edit.selMin()), font).x;
         float selEndX = textStartX + measureText(input.substr(0, _edit.selMax()), font).x;
         Rect<float> selRect = {{selStartX, textPosV}, {selEndX - selStartX, textSize.y}};
         drawRectangle(selRect, theme->foreground.colorHighlight);
      }

      ColorRGBA textColor;
      if (!input.empty() && !disabled){
         textColor = font->color; //normal text, not disabled
      }  else if (!input.empty() && disabled){
         textColor = font->color; //normal text, disabled
      } else if (!_defaultText.empty()){
         textColor = font->colorDisabled; //default text
      }
      drawText(displayText, {textStartX, textPosV}, font, textColor, font->size, font->spacing);
   }

   //draw caret - the handler owns the blink phase and resets it on caret moves/edits
   if (_edit.isEditing() && _edit.caretVisible()) {
      float caretHPos = textStartX + measureText(input.substr(0, _edit.caret()), font).x;
      drawLine({{caretHPos, textPosV}, {caretHPos, textPosV + textSize.y}}, 2, font->color);
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
void LineEdit::setText(const std::string& newText, bool noPublish) {
   _suppressPublish = noPublish; //the write below notifies _on_text_changed synchronously
   _assignString(newText);
   _suppressPublish = false;
   _edit.textReset(); //caret/selection/history offsets are stale against the new text
}

///////////////////////////////////////////////////////////////////////////////////////
void LineEdit::_assignString(const std::string& s) {
   if (getModel()->getText().str() == s) return; //no change: skip the write and the notifications
   TrString next = getModel()->getText(); //copy preserves language + key
   next.assign(s);
   //write through the shared model: publishes EventTextChanged to this and any aliasing view
   getModel()->setText(next);
}

///////////////////////////////////////////////////////////////////////////////////////
void LineEdit::_on_text_changed() {
   //the shared model changed - by our own handler's edit, setText, or another view
   //aliasing the buffer. All publishing funnels through here so every path fires
   //the same old/new virtual + event.
   std::string newText = getModel()->getText().str();
   std::string oldText = _lastText;
   _lastText = newText;
   _edit.externalTextChanged(); //clamp caret/selection if the buffer shrank under them
   _on_text_changed(oldText, newText);
   if (!_suppressPublish) {
      EventLineEditTextChanged event(this, oldText, newText);
      publish(event);
   }
}

///////////////////////////////////////////////////////////////////////////////////////
size_t LineEdit::caretFromMouse(const Pos<float>& localPos) const {
   const auto& input = getModel()->getText().str();
   float adjustedX = localPos.x - TEXT_MARGIN + _scrollOffset;
   if (adjustedX <= 0 || input.empty()) return 0;
   if (adjustedX >= measureText(input, theme->font).x) return input.size(); //past the end
   //getSubstrAt returns the text up to the clicked glyph; its byte length is the caret offset
   return getSubstrAt(input, {adjustedX, 0}, theme->font).size();
}

///////////////////////////////////////////////////////////////////////////////////////
Handled LineEdit::_unhandled_input(const InputEvent& event) {
   //all editing behavior (mouse caret/selection/drag + keyboard) lives in the handler
   if (_edit.handleInput(event)) return this;
   return nullptr;
}

///////////////////////////////////////////////////////////////////////////////////////
void LineEdit::_init() {
   setMaxSize(std::numeric_limits<float>::max(), measureText(_defaultText, theme->font).y);
}

///////////////////////////////////////////////////////////////////////////////////////
void LineEdit::_on_focus_gained() {
   if (!_edit.isEditing()) {
      _edit.setEditing(true);
      //keyboard/tab focus starts at the end; a focusing mouse click immediately
      //overrides this with the clicked position (the handler sets the caret after focusing)
      _edit.setCaret(getModel()->getText().str().size());
      ensureCaretVisible();
   }
}

///////////////////////////////////////////////////////////////////////////////////////
void LineEdit::_on_focus_lost() {
   _edit.setEditing(false);
   _scrollOffset = 0; // Reset scroll when not editing
   _edit.clearSelection();
}

///////////////////////////////////////////////////////////////////////////////////////
void LineEdit::_on_rect_changed() {

}

///////////////////////////////////////////////////////////////////////////////////////
void LineEdit::ensureCaretVisible() {
   const auto& input = getModel()->getText().str();

   // Calculate caret position in text coordinates (without scroll)
   float caretTextPos = measureText(input.substr(0, _edit.caret()), theme->font).x;

   // Calculate visible area
   float visibleWidth = getWidth() - (2 * TEXT_MARGIN);

   // Caret position on screen = caretTextPos - _scrollOffset + TEXT_MARGIN
   float caretScreenPos = caretTextPos - _scrollOffset + TEXT_MARGIN;

   // If caret is off the right edge, scroll right
   if (caretScreenPos > getWidth() - TEXT_MARGIN) {
      _scrollOffset = caretTextPos - visibleWidth;
   }
   // If caret is off the left edge, scroll left
   else if (caretScreenPos < TEXT_MARGIN) {
      _scrollOffset = caretTextPos;
   }

   // Don't scroll past the beginning
   if (_scrollOffset < 0) {
      _scrollOffset = 0;
   }
}
