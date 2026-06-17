#pragma once
#include <algorithm>
#include "TextRenderView.h"

namespace ReyEngine {
   // An editable TextRenderView. Reuses TextRenderView's TextRenderModel<TrString>
   // as the backing store and layers caret/keyboard/mouse editing on top of it,
   // so edits stay localization-aware (the TrString's language/key are preserved).
   class RichTextEditor : public TextRenderView {
   public:
      REYENGINE_OBJECT(RichTextEditor)
      EVENT(EventTextChanged, 6546547654354){}};

      RichTextEditor(auto&&... args): TextRenderView(std::forward<decltype(args)>(args)...){}
      ~RichTextEditor() override = default;

      void setText(const std::string& text); //hides TextRenderModel::setText; preserves language/key
      [[nodiscard]] std::string getString() const {return getText().str();}
      [[nodiscard]] bool isEditing() const {return _isEditing;}
      void clear(){setText("");}
   protected:
      void _init() override;
      void render2D(RenderContext&) const override;
      void _on_focus_gained() override;
      void _on_focus_lost() override;
      Handled _unhandled_input(const InputEvent&) override;
   private:
      Handled _processEdit(const InputEvent&); //real input handling; wrapped by _unhandled_input
      void resetCaretBlink(){_caretBlinkBase = getEngineFrameCount();} //force the caret on, restart the cycle
      void _assignString(const std::string&); //writes back to the model and publishes
      [[nodiscard]] float lineHeight() const;
      // caret <-> (row, col) helpers operating on the current text
      void caretRowCol(const std::string& text, size_t caret, size_t& row, size_t& col) const;
      [[nodiscard]] size_t lineStart(const std::string& text, size_t row) const; //index of first char of row
      [[nodiscard]] size_t caretFromMouse(const Pos<float>& localPos) const;
      void moveCaretVertical(int dir); //dir: -1 up, +1 down
      // UTF-8 boundary navigation: text is stored as UTF-8, so the caret must move
      // a whole codepoint at a time or it splits multibyte characters
      [[nodiscard]] static size_t nextCharBoundary(const std::string& text, size_t i);
      [[nodiscard]] static size_t prevCharBoundary(const std::string& text, size_t i);
      //word-wise navigation (Ctrl+arrows): skip whitespace then a run of word characters
      [[nodiscard]] static size_t nextWordBoundary(const std::string& text, size_t i);
      [[nodiscard]] static size_t prevWordBoundary(const std::string& text, size_t i);
      //convert "\r\n" and lone "\r" to "\n" so stray carriage returns don't render as '?'
      [[nodiscard]] static std::string normalizeNewlines(std::string text);

      // selection helpers (selection spans [selMin, selMax] of the current text)
      [[nodiscard]] bool hasSelection() const {return _selectionAnchor != _caret;}
      [[nodiscard]] size_t selMin() const {return std::min(_selectionAnchor, _caret);}
      [[nodiscard]] size_t selMax() const {return std::max(_selectionAnchor, _caret);}
      void clearSelection(){_selectionAnchor = _caret;} //collapse selection to caret
      [[nodiscard]] std::string getSelectedText() const;
      void deleteSelection(); //erase the selection, place caret at its start, write back
      void drawSelection(float lineHeight) const; //highlight rects, called from render2D

      static constexpr float TEXT_MARGIN = 4.0f;
      size_t _caret = 0;            //number of chars before the caret
      size_t _selectionAnchor = 0;  //fixed end of the selection; == _caret means no selection
      bool _isEditing = false;      //focused & accepting input -> blink the caret
      bool _isDragging = false;     //mouse held down, extending the selection
      EngineFrameCount _caretBlinkBase = 0; //frame the blink cycle started; reset when the caret moves
   };
}
