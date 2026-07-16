#pragma once
#include <algorithm>
#include <functional>
#include <memory>
#include <vector>
#include "InputManager.h"
#include "TextRenderView.h"
#include "WeakUnits.h"

namespace ReyEngine {
   // The editing "brain" shared by every text-editing widget (RichTextEditor, LineEdit).
   // It owns everything about editing that is independent of geometry: the caret and
   // selection, undo/redo history, clipboard operations, UTF-8/word-boundary navigation,
   // and the keyboard/mouse input handling that drives them. All content mutations go
   // through the shared TextRenderModel, so other views aliasing the model stay in sync.
   //
   // Like ScrollView, it is deliberately NOT a Widget: the owning widget composes one as
   // a member and forwards _unhandled_input to handleInput(). Anything geometric (mapping
   // a mouse position to a caret index, moving the caret a visual row up/down, keeping the
   // caret scrolled into view) the handler cannot know - the widget answers those through
   // the Host interface. Names like editFocused are deliberately distinct from Widget's
   // isFocused so a widget can implement Host without hiding/colliding with its own API.
   class TextEditHandler {
   public:
      using Model = TextRenderView::Model;

      struct Host {
         virtual ~Host() = default;
         //geometry: only the widget knows how text is laid out
         [[nodiscard]] virtual size_t caretFromMouse(const Pos<float>& localPos) const = 0;
         //caret moved one visual row up (dir<0) / down (dir>0); single-line hosts return `caret`
         [[nodiscard]] virtual size_t caretVertical(size_t caret, int dir) const = 0;
         [[nodiscard]] virtual size_t rowStart(size_t caret) const = 0; //start of the caret's visual row (Home)
         [[nodiscard]] virtual size_t rowEnd(size_t caret) const = 0;   //end of the caret's visual row (End)
         virtual void ensureCaretVisible() = 0; //scroll so the caret is on-screen
         //widget state the handler consults/drives (distinct names: see class comment)
         [[nodiscard]] virtual EngineFrameCount editFrameCount() const = 0; //for caret blink
         [[nodiscard]] virtual bool editEnabled() const = 0;
         [[nodiscard]] virtual bool editFocused() const = 0;
         virtual void editSetFocused(bool) = 0;
         //notifications
         virtual void onEdited() = 0;                     //a content edit was applied (incl. undo/redo)
         virtual void onContextMenu(const Pos<float>&){}  //right-click released inside the widget
         virtual void onSubmit(){}                        //Enter pressed in single-line mode
         virtual void onEditRejected(const std::string& rejected){}                  //the validator refused an insertion (e.g. beep/flash)
      };

      TextEditHandler(Host& host, std::shared_ptr<Model> model)
      : _host(host), _model(std::move(model)){}

      // Full editing input handling (mouse caret/selection/drag + keyboard). Returns true
      // if the event was consumed. Wraps the actual processing so caret blink, undo
      // coalescing and caret-follow behave uniformly however the caret moved.
      bool handleInput(const InputEvent&);

      //focused-editing state; the widget flips this from its focus callbacks
      void setEditing(bool editing){_editing = editing; if (editing) resetCaretBlink();}
      [[nodiscard]] bool isEditing() const {return _editing;}

      //single-line policy: '\n' never enters the buffer (typing/paste), Enter -> Host::onSubmit
      void setSingleLine(bool singleLine){_singleLine = singleLine;}
      [[nodiscard]] bool isSingleLine() const {return _singleLine;}

      // Optional input validation. The validator is called with the full text the buffer
      // *would* become if a pending insertion were applied; returning false rejects the
      // edit outright (all-or-nothing - a bad paste is dropped, not trimmed). Only
      // insertions (typing/paste/newline) are validated: deletes and undo/redo always
      // pass, so the user can never be trapped in a state they cannot edit out of.
      // Programmatic setText also bypasses it - this validates *input*, not the model.
      using Validator = std::function<bool(const std::string& proposed)>;
      void setValidator(Validator validator){_validator = std::move(validator);} //empty = accept everything
      [[nodiscard]] bool hasValidator() const {return static_cast<bool>(_validator);}
      // Canned Validator matching a fixed-position mask, prefix-wise: partial entry is
      // always accepted, so "192.1" conforms to "###.###.###.###" - whether the field is
      // *complete* is the widget's question to ask at submit time. Mask characters:
      //   '#' digit   'A' letter   'N' letter-or-digit   '*' any codepoint
      //   '\' escapes the next character; anything else is a literal that must match.
      // Character classes are ASCII-only; use '*' (or a custom Validator) for non-ASCII.
      [[nodiscard]] static Validator maskValidator(const std::string& mask);

      // Optional auto-completion companion to the validator. Whenever an insertion is
      // accepted at the *end* of the buffer, the completer is called with the resulting
      // text and returns extra characters to auto-enter (e.g. a mask's literal
      // separators: typing the "192" of "###.###.###.###" auto-enters the '.').
      // It is also consulted in reverse when an end-of-buffer insertion *fails*
      // validation: if auto-entering the completion first makes it pass (the user
      // backspaced an auto-entered literal, then kept typing), the missing run is
      // restored instead of rejecting the keystroke. Mid-buffer edits are never
      // auto-completed. Empty function = no auto-completion.
      using Completer = std::function<std::string(const std::string& textSoFar)>;
      void setCompleter(Completer completer){_completer = std::move(completer);}
      [[nodiscard]] bool hasCompleter() const {return static_cast<bool>(_completer);}
      //companion to maskValidator: auto-enters the mask's literal characters (everything
      //that isn't '#'/'A'/'N'/'*') as soon as the wildcards before them are fulfilled
      [[nodiscard]] static Completer maskCompleter(const std::string& mask);

      // caret/selection queries (for the widget's render code)
      [[nodiscard]] size_t caret() const {return _caret;}
      //place the caret programmatically (clamped): collapses the selection, restarts the blink
      void setCaret(size_t pos){_caret = std::min(pos, text().size()); clearSelection(); resetCaretBlink();}
      [[nodiscard]] bool hasSelection() const {return _selectionAnchor != _caret;}
      [[nodiscard]] size_t selMin() const {return std::min(_selectionAnchor, _caret);}
      [[nodiscard]] size_t selMax() const {return std::max(_selectionAnchor, _caret);}
      [[nodiscard]] std::string getSelectedText() const;
      [[nodiscard]] bool caretVisible() const; //blink phase; resets whenever the caret moves

      // undo/redo
      void undo(); //reverse the most recent edit
      void redo(); //re-apply the most recently undone edit
      [[nodiscard]] bool canUndo() const {return _undoCursor > 0;}
      [[nodiscard]] bool canRedo() const {return _undoCursor < _undo.size();}

      // clipboard/selection operations (also used by context menus / external callers)
      void selectAll(){_selectionAnchor = 0; _caret = text().size();}
      void clearSelection(){_selectionAnchor = _caret;} //collapse selection to caret
      void copySelection() const; //copy the selection to the clipboard (no-op without a selection)
      void pasteClipboard();      //paste clipboard text over the selection (no-op if clipboard empty)
      void deleteSelection();     //erase the selection (as a RemoveTextAction), caret at its start

      // The widget must call these when text changes outside the handler's own actions:
      // externalTextChanged() on any model notification (a remote view may have shrunk the
      // buffer under our caret); textReset() on wholesale replacement (setText) - offsets
      // recorded in the history are invalid against the new text.
      void externalTextChanged(); //clamp caret/anchor into the (possibly smaller) buffer
      void textReset();           //clamp caret, drop selection + history, restart blink

      // UTF-8 boundary navigation: text is stored as UTF-8, so the caret must move a whole
      // codepoint at a time or it splits multibyte characters. Public: hosts need these for
      // their own geometry (wrap layout, vertical caret motion).
      [[nodiscard]] static size_t nextCharBoundary(const std::string& text, size_t i);
      [[nodiscard]] static size_t prevCharBoundary(const std::string& text, size_t i);
      //word-wise navigation (Ctrl+arrows): skip whitespace then a run of word characters
      [[nodiscard]] static size_t nextWordBoundary(const std::string& text, size_t i);
      [[nodiscard]] static size_t prevWordBoundary(const std::string& text, size_t i);
      //convert "\r\n" and lone "\r" to "\n" so stray carriage returns don't render as '?'
      [[nodiscard]] static std::string normalizeNewlines(std::string text);

   private:
      [[nodiscard]] const std::string& text() const {return _model->getText().str();}
      bool processEdit(const InputEvent&); //real input handling; wrapped by handleInput
      void resetCaretBlink(){_caretBlinkBase = _host.editFrameCount();} //force the caret on, restart the cycle
      void afterEdit(); //post-mutation bookkeeping shared by every action: blink + Host::onEdited

      //Any manipulation of the contents must be performed by an edit action. These must
      //have an inverse operation so they can be undone. The handler never mutates the
      //model directly: it builds an EditAction, calls redo() to apply it, and pushes it
      //onto the history. undo() must exactly reverse redo(). redo()/undo() receive the
      //handler so actions stay lightweight, carrying only their own data.
      struct EditAction {
         virtual ~EditAction() = default;
         virtual void redo(TextEditHandler&) = 0;
         virtual void undo(TextEditHandler&) = 0;
         //Try to fold `next` (a just-created action) into this one so a burst of
         //related edits collapses to a single undo step. Return true if absorbed
         //(the caller then discards `next`); false to keep them as separate steps.
         //Default: actions never merge. The push layer decides *whether* to attempt
         //a merge (e.g. only within a typing run); this judges *whether it can*.
         virtual bool tryMerge(const EditAction& /*next*/){ return false; }
      };

      struct InsertTextAction : public EditAction {
         size_t index;   //caret position the text was inserted at
         TrString text;  //the run that was inserted
         InsertTextAction(size_t index, TrString text): index(index), text(std::move(text)){}
         void redo(TextEditHandler&) override; //insert `text` at `index`
         void undo(TextEditHandler&) override; //remove the inserted run
         bool tryMerge(const EditAction&) override; //coalesce contiguous typing
      };

      struct RemoveTextAction : public EditAction {
         size_t index;   //offset the run was removed from
         TrString text;  //the run that was removed (captured so undo can restore it)
         RemoveTextAction(size_t index, TrString text): index(index), text(std::move(text)){}
         void redo(TextEditHandler&) override; //remove the run at `index`
         void undo(TextEditHandler&) override; //re-insert the removed run
         bool tryMerge(const EditAction&) override; //coalesce consecutive deletes
      };

      //Groups several actions into a single undoable step (e.g. replace-selection
      //= remove + insert, or indenting many lines at once). redo() replays the
      //children front-to-back; undo() reverses them back-to-front.
      struct CompositeAction : public EditAction {
         std::vector<std::shared_ptr<EditAction>> actions;
         void redo(TextEditHandler&) override;
         void undo(TextEditHandler&) override;
      };

      // undo/redo driver. Every content change funnels through pushAction: it applies
      // the action now, drops any stale redo branch, optionally coalesces it into the
      // previous step, and records it. replaceSelectionWith() is the shared path for
      // edits that overwrite a selection (type/paste/newline over a highlight).
      void pushAction(std::shared_ptr<EditAction> action, bool mergeable);
      void replaceSelectionWith(const TrString& text, bool mergeable);
      void breakUndoMerge(){_coalesce = false;} //a caret move/discrete op ends a typing run
      void resetHistory(){_undo.clear(); _undoCursor = 0; _coalesce = false;}

      Host& _host;
      std::shared_ptr<Model> _model;
      size_t _caret = 0;            //number of bytes before the caret
      size_t _selectionAnchor = 0;  //fixed end of the selection; == _caret means no selection
      bool _editing = false;        //focused & accepting input -> blink the caret
      bool _isDragging = false;     //mouse held down, extending the selection
      bool _singleLine = false;     //reject newlines; Enter submits instead of inserting
      Validator _validator;         //insertions must pass this to be applied; empty = no validation
      Completer _completer;         //auto-enters due characters around end-of-buffer insertions
      EngineFrameCount _caretBlinkBase = 0; //frame the blink cycle started; reset when the caret moves

      // Command history for undo/redo. Actions [0, _undoCursor) are currently applied;
      // [_undoCursor, size) are undone and available to redo. A fresh edit truncates
      // the redo tail.
      std::vector<std::shared_ptr<EditAction>> _undo;
      size_t _undoCursor = 0;  //count of actions currently applied
      bool _coalesce = false;  //may the next mergeable edit fold into the last one?
      bool _didEdit = false;   //set by pushAction; lets handleInput tell edits from caret moves
   };
}
