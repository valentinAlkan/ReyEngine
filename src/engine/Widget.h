#pragma once
#include "Drawable2D.h"
#include "InputHandler.h"
#include "Processable.h"
#include "Theme.h"

namespace ReyEngine {
   enum class Anchor{NONE, LEFT, RIGHT, TOP, BOTTOM, FILL, TOP_LEFT, TOP_RIGHT, BOTTOM_RIGHT, BOTTOM_LEFT, CENTER};
   class Widget
   : public Internal::Drawable2D
   , public Internal::Processable
   , public EventPublisher
   , public EventSubscriber
   {
   public:
      EVENT_ARGS(ResizeEvent, 329874, const Rect<float>& r)
      , rect(r)
      {}
         Rect<float> rect;
      };
      Widget(): theme(new Theme){}
      REYENGINE_OBJECT(Widget)
      Theme& getTheme(){return *theme;}
      void setAnchoring(Anchor newAnchor);
      Anchor getAnchoring() const {return _anchor;}
      bool isHovered() const;
   protected:
      //input
      virtual Handled _unhandled_input(const InputEvent&){return false;} //pass input to children if they want it and then process it for ourselves if necessary
      virtual Handled __process_unhandled_input(const InputEvent& event){ return _unhandled_input(event);}
      virtual Handled _process_unhandled_editor_input(const InputEvent&){return false;} //pass input to children if they want it and then process it for ourselves if necessary ONLY FOR EDITOR RELATED THINGS (grab handles mostly)
      InputFilter _inputFilter = InputFilter::INPUT_FILTER_PASS_AND_PROCESS;
      virtual void _on_mouse_enter(){};
      virtual void _on_mouse_exit(){};
      virtual void _on_modality_gained(){}
      virtual void _on_modality_lost(){}
      virtual void _on_focus_gained(){}
      virtual void _on_focus_lost(){}
      void _process(float dt) override {};
      bool acceptsHover = false;
      bool isLayout = false;
      bool enabled = true; //changes visuals and (typically) ingores input

      RefCounted<Theme> theme;

      void __on_rect_changed(const Rect<R_FLOAT>& oldRect, bool byLayout = false) override {
         if (!byLayout) {
            //layout will have already resized children
            calculateAnchoring(getRect());
         }
         _on_rect_changed();
         auto event = ResizeEvent(this, oldRect);
         publish(event);

         //try to inform children of resize in case they're anchored or need to do other logic
         for (auto& child: getChildren()) {
            if (auto isWidget = child->as<Widget>()) {
               auto& childWidget = isWidget.value();
               isWidget.value()->setRect(childWidget->getRect());
            }
         }

         //might not be in tree yet.
         if (!getNode()) return;
         if (byLayout) return; //layouts don't inform their parents when the child rect is changed
         if (auto parent = getNode()->getParent()) {
            if (parent) {
               if (auto parentIsWidget = parent->as<Widget>()) {
                  parentIsWidget.value()->_on_child_rect_changed();
               }
            }
         }
      }
      void calculateAnchoring(const Rect<R_FLOAT>& oldRect);
      virtual void _on_child_rect_changed(){};
      virtual void _on_rect_changed(){};
      Anchor _anchor = Anchor::NONE;
      friend class Layout;
      friend class Canvas;
   };
}