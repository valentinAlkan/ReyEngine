#pragma once
#include "TextEditHandler.h"
#include "TextRenderView.h"

namespace ReyEngine {
   // A single-line text input. Like RichTextEditor it is a TextRenderView: the text
   // lives in a shared TextRenderModel<TrString> (so it can alias/observe the same
   // buffer as other views), and all editing behavior (caret/selection, undo/redo,
   // clipboard, keyboard+mouse input) comes from the composed TextEditHandler, run
   // in single-line mode: newlines never enter the buffer and Enter publishes
   // EventLineEditTextEntered instead of inserting a line break. This class owns
   // only what is specific to a one-line box: the placeholder ("default") text,
   // horizontal caret-follow scrolling, and rendering.
   class LineEdit : public TextRenderView, private TextEditHandler::Host {
   public:
      REYENGINE_OBJECT(LineEdit)
      EVENT_ARGS(EventDefaultTextChanged, 754321525, const std::string& oldText, const std::string& newText)
      , oldText(oldText)
      , newText(newText)
      {}
         const std::string oldText;
         const std::string newText;
      };
      EVENT_ARGS(EventTextChanged, 754321526, const std::string& oldText, const std::string& newText)
      , oldText(oldText)
      , newText(newText)
      {}
         const std::string oldText;
         const std::string newText;
      };
      EVENT_ARGS(EventInputRejected, 4465498432468, const std::string& rejectedText)
         , rejectedText(rejectedText)
         {}
         const std::string rejectedText;
      };
      EVENT(EventTextEntered, 754321527){}};

      LineEdit(const std::string& defaultText = "Default")
      : TextRenderView(std::make_shared<TrString>())
      , _edit(*this, getModel())
      , _defaultText(defaultText)
      {
         _edit.setSingleLine(true); //'\n' can never enter the buffer; Enter -> onSubmit
      }
      //point at an existing view's model: same buffer, same notifications (edits made
      //here appear in every other view aliasing the model, and vice versa)
      explicit LineEdit(const std::shared_ptr<TextRenderView>& other)
      : TextRenderView(other)
      , _edit(*this, getModel())
      , _lastText(getModel()->getText().str()) //buffer may already have content
      {
         _edit.setSingleLine(true);
      }

      inline void clear(){ setText("");}
      void setText(const std::string&, bool noPublish=false);
      void setDefaultText(const std::string&, bool noPublish=false);
      void render2D(RenderContext&) const override;
      //shows the placeholder when the input is empty (historical LineEdit behavior;
      //hides TextRenderView::getText, which returns the raw stored TrString)
      [[nodiscard]] std::string getText() const {
         const auto& input = getModel()->getText().str();
         return input.empty() ? _defaultText : input;
      }
      [[nodiscard]] bool isEditing() const {return _edit.isEditing();}
      //optional input validation (see TextEditHandler::setValidator for the contract);
      //existing text is not re-validated - only subsequent insertions are checked.
      //Replaces any mask set via setInputMask, including its auto-completion.
      void setValidator(TextEditHandler::Validator validator){
         _edit.setValidator(std::move(validator));
         _edit.setCompleter({}); //a leftover mask completer would fight the new validator
      }
      //Restrict input to a fixed-position mask, e.g. "###.###.###.###" (IPv4) or
      //"AA-####" (part number); "" removes the mask.
      //Mask characters:
      //  '#' digit
      //  'A' letter
      //  'N' letter-or-digit
      //  '*' any character
      //  '\' escapes the next character; anything else is a literal the user must type.
      //Matching is prefix-wise: partial entry is accepted while typing, so check the
      //text is complete (full mask length) at submit time. Classes are ASCII-only.
      //With autocomplete on, literals enter themselves as soon as the wildcards before
      //them are fulfilled: typing the "192" of "###.###.###.###" auto-enters the '.'.
      void setInputMask(const std::string& mask, bool autocomplete = true){
         _edit.setValidator(mask.empty() ? TextEditHandler::Validator{} : TextEditHandler::maskValidator(mask));
         _edit.setCompleter(mask.empty() || !autocomplete ? TextEditHandler::Completer{} : TextEditHandler::maskCompleter(mask));
      }
   protected:
      void _init() override;
      void _on_focus_gained() override;
      void _on_focus_lost() override;
      void _on_rect_changed() override;
      Handled _unhandled_input(const InputEvent&) override;
      virtual void _on_default_text_changed(const std::string& old, const std::string& _new){};
      virtual void _on_text_changed(const std::string& old, const std::string& _new){};
      void onEditRejected(const std::string& rejected) override {publish(EventInputRejected(this, rejected));}
   private:
      void _on_text_changed() override; //shared model changed: fire the old/new virtual + event

      // TextEditHandler::Host: geometry/state answers for the editing logic
      [[nodiscard]] size_t caretFromMouse(const Pos<float>& localPos) const override;
      [[nodiscard]] size_t caretVertical(size_t caret, int) const override {return caret;} //single line: up/down don't move
      [[nodiscard]] size_t rowStart(size_t) const override {return 0;} //Home: start of the line
      [[nodiscard]] size_t rowEnd(size_t) const override {return getModel()->getText().str().size();} //End: end of the line
      void ensureCaretVisible() override; //adjusts _scrollOffset so the caret is visible
      [[nodiscard]] EngineFrameCount editFrameCount() const override {return getEngineFrameCount();}
      [[nodiscard]] bool editEnabled() const override {return getIsEnabled();}
      [[nodiscard]] bool editFocused() const override {return isFocused();}
      void editSetFocused(bool focused) override {setFocused(focused);}
      void onEdited() override {} //publishing rides the model notification (_on_text_changed) instead
      void onSubmit() override {publish(EventTextEntered(this));} //Enter pressed

      void _assignString(const std::string&); //write to the model preserving the TrString's language/key

      static constexpr float TEXT_MARGIN = 4.0f;
      TextEditHandler _edit;    //caret/selection/undo/clipboard + edit input, in single-line mode
      std::string _defaultText; //placeholder shown while the input is empty
      std::string _lastText;    //previous content; provides oldText for the changed event
      bool _suppressPublish = false; //setText(noPublish=true): run virtuals but don't publish
      float _scrollOffset = 0;  //horizontal scroll offset for text
   };
}
