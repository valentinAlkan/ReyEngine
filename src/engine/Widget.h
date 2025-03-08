#pragma once
#include "Drawable2D.h"
#include "InputHandler.h"
#include "Processable.h"
#include "Theme.h"

namespace ReyEngine {
   enum class Anchor{NONE, LEFT, RIGHT, TOP, BOTTOM, FILL, TOP_LEFT, TOP_RIGHT, BOTTOM_RIGHT, BOTTOM_LEFT, CENTER};
   class Widget
   : public Internal::Drawable2D
   , public Internal::InputHandler
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
      Anchor getAnchoring(){return _anchor;}
   protected:
      Handled _unhandled_input(const InputEvent& event) override {return false;}
      void _process(float dt) override {};

      bool acceptsHover = false;

      bool isLayout = false;
      RefCounted<Theme> theme;

      void __on_rect_changed(const Rect<R_FLOAT>& oldRect) override {
         calculateAnchoring(getRect());
         _on_rect_changed();
         auto event = ResizeEvent(this, oldRect);
         publish(event);

         //try to inform children of resize in case they're anchored
         for (auto& child : getChildren()) {
            if (auto isWidget = child->as<Widget>()) {
               auto& childWidget = isWidget.value();
               isWidget.value()->setRect(childWidget->getRect());
            }
         }

         //might not be in tree yet.
         if (!getNode()) return;
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
   };
}