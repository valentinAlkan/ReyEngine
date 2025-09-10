#pragma once
#include "Widget.h"

namespace ReyEngine {
   class LineEdit : public Widget {
   public:
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

      REYENGINE_OBJECT(LineEdit)

      LineEdit(const std::string& defaultText = "Default")
      : _defaultText(defaultText)
      {
         setMaxSize(std::numeric_limits<float>::max(), measureText(defaultText, theme->font).y);
      }

      inline void clear(){ setText("");}
      void setText(const std::string&, bool noPublish=false);
      void setDefaultText(const std::string&, bool noPublish=false);
      void render2D() const override;
      std::string getText(){return _text;}
   protected:
      void _on_focus_gained() override;
      void _on_focus_lost() override;
      void _on_rect_changed() override;
      Widget* _unhandled_input(const InputEvent&) override;
      virtual void _on_default_text_changed(const std::string& old, const std::string& _new){};
      virtual void _on_text_changed(const std::string& old, const std::string& _new){};
   private:
      void publishText(const std::string&);
      std::string _defaultText;
      std::string _text;
      Rect<int> _scissorArea;
      int _highlight_start = 0;
      int _highlight_end = 0;
      bool _isEditing = false; //the user is editing - blink the cursor
      int _caretPos = 0; //the char BEFORE which the cursor should be drawn; -1 if end
   };
}