#pragma once
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
      void _assignString(const std::string&); //writes back to the model and publishes
      [[nodiscard]] float lineHeight() const;
      // caret <-> (row, col) helpers operating on the current text
      void caretRowCol(const std::string& text, size_t caret, size_t& row, size_t& col) const;
      [[nodiscard]] size_t lineStart(const std::string& text, size_t row) const; //index of first char of row
      [[nodiscard]] size_t caretFromMouse(const Pos<float>& localPos) const;
      void moveCaretVertical(int dir); //dir: -1 up, +1 down

      static constexpr float TEXT_MARGIN = 4.0f;
      size_t _caret = 0;       //number of chars before the caret
      bool _isEditing = false; //focused & accepting input -> blink the caret
   };
}
