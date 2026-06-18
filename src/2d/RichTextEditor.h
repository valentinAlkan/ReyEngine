#pragma once
#include <algorithm>
#include <cstdint>
#include <memory>
#include <vector>
#include "History.h"
#include "ScrollView.h"
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
   void undo(); //reverse the most recent edit
   void redo(); //re-apply the most recently undone edit
   [[nodiscard]] bool canUndo() const {return _undoCursor > 0;}
   [[nodiscard]] bool canRedo() const {return _undoCursor < _undo.size();}
   //word wrap: when enabled, logical lines too wide for the widget are broken across
   //multiple visual rows at word boundaries. The stored text is unchanged; only layout differs.
   void setWordWrapEnabled(bool enabled){_wordWrap = enabled;}
   [[nodiscard]] bool getWordWrapEnabled() const {return _wordWrap;}
   protected:
   void _init() override;
   void _process(float dt) override; //per-frame: keep the scroll limits and bar in sync with content
   void render2D(RenderContext&) const override;
   void _on_focus_gained() override;
   void _on_focus_lost() override;
   Handled _unhandled_input(const InputEvent&) override;
   private:
   Handled _processEdit(const InputEvent&); //real input handling; wrapped by _unhandled_input
   void resetCaretBlink(){_caretBlinkBase = getEngineFrameCount();} //force the caret on, restart the cycle
   void _assignString(const std::string&); //writes back to the model and publishes
   [[nodiscard]] float lineHeight() const;

   // A visual row of on-screen text: the byte range [start, end) of `text` drawn on
   // one line. Excludes any trailing '\n'. With word wrap off there is one VisualLine
   // per logical line; with it on, wide logical lines split into several.
   struct VisualLine { size_t start; size_t end; };
   [[nodiscard]] std::vector<VisualLine> computeVisualLines(const std::string& text) const;
   //cached wrapper around computeVisualLines: recomputes only when the text, width,
   //wrap flag or font changes, so layout isn't rebuilt every frame.
   [[nodiscard]] const std::vector<VisualLine>& visualLines() const;
   [[nodiscard]] float wrapWidth() const;     //usable text width inside the margins (minus scrollbar gutter)
   [[nodiscard]] float viewportWidth() const; //widget width minus the scrollbar gutter when the bar is shown
   [[nodiscard]] float contentHeight() const; //total height of all visual rows plus top/bottom margins
   void updateScrollLayout();                 //refresh scroll limits + bar from current content/viewport
   void ensureCaretVisible();                 //scroll vertically so the caret's row sits in the viewport
   //find the visual row containing `caret` and the byte column within it
   static void rowColForCaret(const std::vector<VisualLine>& lines, size_t caret, size_t& row, size_t& col);

   // caret <-> (row, col) helpers operating on the current text (wrap-aware via VisualLine)
   void caretRowCol(const std::string& text, size_t caret, size_t& row, size_t& col) const;
   [[nodiscard]] size_t lineStart(const std::string& text, size_t row) const; //start byte of visual row
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
   void deleteSelection(); //erase the selection (as a RemoveTextAction), caret at its start
   void drawSelection(const std::vector<VisualLine>& lines, float lineHeight) const; //highlight rects, called from render2D

   // undo/redo driver. Every content change funnels through pushAction: it applies
   // the action now, drops any stale redo branch, optionally coalesces it into the
   // previous step, and records it. replaceSelectionWith() is the shared path for
   // edits that overwrite a selection (type/paste/newline over a highlight).
   struct EditAction; //the action hierarchy is defined further down in this class
   void pushAction(std::shared_ptr<EditAction> action, bool mergeable);
   void replaceSelectionWith(const TrString& text, bool mergeable);
   void breakUndoMerge(){_coalesce = false;} //a caret move/discrete op ends a typing run
   void resetHistory(){_undo.clear(); _undoCursor = 0; _coalesce = false;}

   static constexpr float TEXT_MARGIN = 4.0f;
   size_t _caret = 0;            //number of chars before the caret
   size_t _selectionAnchor = 0;  //fixed end of the selection; == _caret means no selection
   bool _isEditing = false;      //focused & accepting input -> blink the caret
   bool _isDragging = false;     //mouse held down, extending the selection
   bool _wordWrap = false;       //wrap wide logical lines across visual rows
   EngineFrameCount _caretBlinkBase = 0; //frame the blink cycle started; reset when the caret moves

   // vertical scrolling. _scroll owns the offset/limits and keeps _vScrollBar in sync;
   // the editor offsets its own text drawing and translates input by _scroll.offsetY().
   ScrollView _scroll;
   std::shared_ptr<Slider> _vScrollBar;
   static constexpr float SCROLLBAR_WIDTH = 14.0f;
   uint64_t _textVersion = 1;    //bumped on every content change; keys the layout cache

   // layout cache (mutable: filled lazily from const render/query paths). Rebuilt only
   // when one of the keys below no longer matches the current text/width/wrap/font.
   mutable std::vector<VisualLine> _layoutCache;
   mutable bool _layoutValid = false;
   mutable uint64_t _layoutTextVersion = 0;
   mutable float _layoutWidth = -1.0f;
   mutable bool _layoutWrap = false;
   mutable float _layoutFontSize = -1.0f;
   mutable float _layoutFontSpacing = -1.0f;


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
      InsertTextAction(size_t index, TrString text): index(index), text(std::move(text)){}
      void redo(RichTextEditor&) override; //insert `text` at `index`
      void undo(RichTextEditor&) override; //remove the inserted run
      bool tryMerge(const EditAction&) override; //coalesce contiguous typing
   };

   struct RemoveTextAction : public EditAction {
      size_t index;   //offset the run was removed from
      TrString text;  //the run that was removed (captured so undo can restore it)
      RemoveTextAction(size_t index, TrString text): index(index), text(std::move(text)){}
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

   // Command history for undo/redo. Actions [0, _undoCursor) are currently applied;
   // [_undoCursor, size) are undone and available to redo. A fresh edit truncates
   // the redo tail. (History<T> isn't used here: its back()/fwd() are browser-style
   // state navigation and can't express "undo the action at the cursor".)
   std::vector<std::shared_ptr<EditAction>> _undo;
   size_t _undoCursor = 0;  //count of actions currently applied
   bool _coalesce = false;  //may the next mergeable edit fold into the last one?
   bool _didEdit = false;   //set by pushAction; lets _unhandled_input tell edits from caret moves
   };
}
