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

   REYENGINE_OBJECT(LineEdit, BaseWidget)
   , PROPERTY_DECLARE(_defaultText)
   , PROPERTY_DECLARE(_text)
   {}
   public:
      void setText(const std::string&);
      void setDefaultText(const std::string&);
      void render() const override;
      void registerProperties() override;
   protected:
      Handled _unhandled_input(const InputEvent&, const std::optional<UnhandledMouseInput>&) override;
      virtual void _on_default_text_changed(const std::string &){};
      virtual void _on_text_changed(const std::string &){};
   private:
      StringProperty _defaultText;
      StringProperty _text;
      bool _cursorBlinkOn = false; //true when the cursor should be displayed (like, actually drawn)
      bool _highlight = false; //draw the highlight
      bool _isEditing = false; //the user is editing
   };
}