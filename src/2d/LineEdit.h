#pragma once
#include "Widget.h"

namespace ReyEngine {
   class LineEdit : public Widget {
   public:
      REYENGINE_OBJECT(LineEdit)
      EVENT_ARGS(EventLineEditDefaultTextChanged, 754321525, const std::string& oldText, const std::string& newText)
      , oldText(oldText)
      , newText(newText)
      {}
         const std::string oldText;
         const std::string newText;
      };
      EVENT_ARGS(EventLineEditTextChanged, 754321526, const std::string& oldText, const std::string& newText)
      , newText(newText)
      , oldText(oldText)
      {}
         const std::string oldText;
         const std::string newText;
      };
      EVENT(EventLineEditTextEntered, 754321527){}};

      LineEdit(const std::string& defaultText = "Default")
      : _defaultText(defaultText)
      {}

      inline void clear(){ setText("");}
      void setText(const std::string&, bool noPublish=false);
      void setDefaultText(const std::string&, bool noPublish=false);
      void render2D() const override;
      [[nodiscard]] std::string getText(){return _input.empty() ? _defaultText : _input;}
   protected:
      void _init() override;
      void _on_focus_gained() override;
      void _on_focus_lost() override;
      void _on_rect_changed() override;
      Widget* _unhandled_input(const InputEvent&) override;
      virtual void _on_default_text_changed(const std::string& old, const std::string& _new){};
      virtual void _on_text_changed(const std::string& old, const std::string& _new){};
   private:
      void publishText(const std::string&);
      void ensureCaretVisible(); //adjusts _scrollOffset so caret is visible

      // Selection helpers
      bool hasSelection() const { return _selectionStart != _selectionEnd; }
      void clearSelection() { _selectionStart = _selectionEnd = 0; }
      std::string getSelectedText() const;
      void deleteSelection(); //deletes selected text and updates caret
      void setSelectionFromCaret(); //sets selection start to current caret pos
      int getSelectionMin() const;
      int getSelectionMax() const;
      int caretPosToIndex(int pos) const; //converts -1 to actual index
      int getCaretPosFromMouse(float mouseLocalX) const; //gets caret position from mouse x coordinate

      std::string _defaultText;
      std::string _input;
      int _selectionStart = 0; //character index where selection begins
      int _selectionEnd = 0;   //character index where selection ends (can be < start)
      bool _isEditing = false; //the user is editing - blink the cursor
      bool _isDragging = false; //mouse is being dragged for selection
      int _caretPos = 0; //the char BEFORE which the cursor should be drawn; -1 if end
      float _scrollOffset = 0; //horizontal scroll offset for text
      std::chrono::steady_clock::time_point caretHighTime; //timestamp of when the caret should be high
   };
}