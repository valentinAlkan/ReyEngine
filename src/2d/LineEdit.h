#pragma once
#include "BaseWidget.h"

namespace ReyEngine {
   class LineEdit : public BaseWidget {
   public:
      struct EventLineEditDefaultTextChanged : public Event<EventLineEditDefaultTextChanged>{
         EVENT_CTOR_SIMPLE(EventLineEditDefaultTextChanged, Event<EventLineEditDefaultTextChanged>, const std::string& newText), newText(newText){}
         const std::string newText;
      };
      struct EventLineEditTextChanged : public Event<EventLineEditTextChanged>{
         EVENT_CTOR_SIMPLE(EventLineEditTextChanged, Event<EventLineEditTextChanged>, const std::string& newText), newText(newText){}
         const std::string newText;
      };

   REYENGINE_OBJECT_BUILD_ONLY(LineEdit, BaseWidget)
   , PROPERTY_DECLARE(_defaultText)
   , PROPERTY_DECLARE(_text)
   {}
   public:
      REYENGINE_DEFAULT_BUILD(LineEdit)
      inline void clear(){ setText("");}
      void setText(const std::string&, bool noPublish=false);
      void setDefaultText(const std::string&, bool noPublish=false);
      void render2D() const override;
      void registerProperties() override;
      std::string getText(){return _text;}
   protected:
      void _on_focus_gained() override;
      void _on_focus_lost() override;
      void _on_rect_changed() override;
      Handled _unhandled_input(const InputEvent&, const std::optional<UnhandledMouseInput>&) override;
      virtual void _on_default_text_changed(const std::string &){};
      virtual void _on_text_changed(const std::string &){};
   private:
      void publishText();
      StringProperty _defaultText;
      StringProperty _text;
      Rect<int> _scissorArea;
      int _highlight_start = 0;
      int _highlight_end = 0;
      bool _isEditing = false; //the user is editing - blink the cursor
      int _caretPos = 0; //the char BEFORE which the cursor should be drawn; -1 if end
   };
}