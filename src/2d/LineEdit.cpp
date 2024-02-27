#include "LineEdit.h"

using namespace std;
using namespace ReyEngine;

///////////////////////////////////////////////////////////////////////////////////////
void LineEdit::render() const {
   _drawRectangle(_rect.value.toSizeRect(), theme->background.colorPrimary);

   auto& font = theme->font;
   auto textheight = font.value.size;
   //available vertical height
   auto availableHeight = _rect.value.height;
   auto textPosV = (int)((availableHeight - textheight) / 2);

   //draw default text
   if (_text.value.empty() && !_defaultText.value.empty()){
      if (_highlight){
         auto highlightRect = _rect.value - Size<int>(2,2) + Pos<int>(1,1);
         _drawRectangle(highlightRect, theme->highlight);
      }
      _drawText(_defaultText, {1,textPosV}, font);
   }
}

///////////////////////////////////////////////////////////////////////////////////////
void LineEdit::registerProperties() {
   registerProperty(_defaultText);
   registerProperty(_text);
}

///////////////////////////////////////////////////////////////////////////////////////
void LineEdit::setDefaultText(const std::string& _newDefaultText) {
   _defaultText = _newDefaultText;
   _on_default_text_changed(_defaultText);
   EventLineEditDefaultTextChanged event(toEventPublisher(), _defaultText);
   publish(event);
}

///////////////////////////////////////////////////////////////////////////////////////
void LineEdit::setText(const std::string& _newText) {
   _text = _newText;
   _on_text_changed(_text);
   EventLineEditTextChanged event(toEventPublisher(), _defaultText);
   publish(event);
}

///////////////////////////////////////////////////////////////////////////////////////
Handled LineEdit::_unhandled_input(const InputEvent& event, const std::optional<UnhandledMouseInput>& mouse) {
   return false;
}