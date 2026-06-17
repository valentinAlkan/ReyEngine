#pragma once
#include <algorithm>
#include <memory>
#include <vector>
#include "History.h"
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


   //Any manipulation of the contents of the editor must be performed by an edit action.
   // These must have an inverse operation so they can be undone. The editor never
   // mutates its own contents directly: it builds an EditAction, calls redo() to
   // apply it, and pushes it onto the history. undo() must exactly reverse redo().
   // redo()/undo() receive the editor so actions stay lightweight, carrying only
   // their own data rather than a back-pointer to the editor.
   struct EditAction {
      virtual ~EditAction() = default;
      virtual void redo(RichTextEditor&) = 0;
      virtual void undo(RichTextEditor&) = 0;
      //Try to fold `next` (a just-created action) into this one so a burst of
      //related edits collapses to a single undo step. Return true if absorbed
      //(the caller then discards `next`); false to keep them as separate steps.
      //Default: actions never merge. The push layer decides *whether* to attempt
      //a merge (e.g. only within a short time window); this judges *whether it can*.
      virtual bool tryMerge(const EditAction& /*next*/){ return false; }
   };

   struct InsertTextAction : public EditAction {
      size_t index;   //caret position the text was inserted at
      TrString text;  //the run that was inserted
      void redo(RichTextEditor&) override; //insert `text` at `index`
      void undo(RichTextEditor&) override; //remove the inserted run
      bool tryMerge(const EditAction&) override; //coalesce contiguous typing
   };

   struct RemoveTextAction : public EditAction {
      size_t index;   //offset the run was removed from
      TrString text;  //the run that was removed (captured so undo can restore it)
      void redo(RichTextEditor&) override; //remove the run at `index`
      void undo(RichTextEditor&) override; //re-insert the removed run
      bool tryMerge(const EditAction&) override; //coalesce consecutive deletes
   };

   //Groups several actions into a single undoable step (e.g. replace-selection
   //= remove + insert, or indenting many lines at once). redo() replays the
   //children front-to-back; undo() reverses them back-to-front.
   struct CompositeAction : public EditAction {
      std::vector<std::shared_ptr<EditAction>> actions;
      void redo(RichTextEditor&) override;
      void undo(RichTextEditor&) override;
   };

   //holds the undo history
   History<std::shared_ptr<EditAction>> _history;
   };
}
