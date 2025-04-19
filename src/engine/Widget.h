#pragma once
#include "Drawable2D.h"
#include "InputHandler.h"
#include "Processable.h"
#include "Theme.h"
#include "WeakUnits.h"
#include "MetaData.h"

namespace ReyEngine {
   enum class Anchor{NONE, LEFT, RIGHT, TOP, BOTTOM, FILL, TOP_LEFT, TOP_RIGHT, BOTTOM_RIGHT, BOTTOM_LEFT, CENTER};
   class Widget
   : public Internal::Drawable2D
   , public Internal::Processable
   , public EventPublisher
   , public EventSubscriber
   , public MetaDataInterface
   {
   public:
      EVENT_ARGS(RectChangedEvent, 329874, const Rect<float>& r)
      , rect(r)
      {}
         Rect<float> rect;
      };

      EVENT_ARGS(WidgetUnhandledInputEvent, 329875, const InputEvent& fwdEvent)
         , fwdEvent(fwdEvent)
         {}
         Rect<R_FLOAT> rect;
         const InputEvent& fwdEvent;
         bool handled = false;
      };

      Widget(): theme(new Theme){
      }
      ~Widget(){
         if constexpr (isDebugBuild) {
            std::cout << "Goodbye from " << _name << "!!" << std::endl;
         }
      }
      REYENGINE_OBJECT(Widget)
      Theme& getTheme(){return *theme;}
      std::optional<Widget*> getParentWidget() const;
      void setAnchoring(Anchor newAnchor);
      Rect<R_FLOAT> getChildBoundingBox() const;
      [[nodiscard]] Anchor getAnchoring() const {return _anchor;}
      [[nodiscard]] FrameCount getFrameCount() const;
      [[nodiscard]] bool isHovered() const;
      [[nodiscard]] bool isFocused() const;
      [[nodiscard]] bool isModal() const;
      void setHovered(bool);
      void setFocused(bool);
      void setModal(bool);
      Pos<float> getLocalMousePos() const;
      CanvasSpace<Pos<float>> toCanvasSpace(const Pos<float>&);
      WindowSpace<Pos<float>> toWindowSpace(const Pos<float>&);
      virtual Widget* _unhandled_input(const InputEvent&);
      void setEnabled(bool newState){enabled = newState;}
      bool getIsEnabled() const {return enabled;}
   protected:
      //input
      virtual Widget* __process_unhandled_input(const InputEvent& event){ return _unhandled_input(event);}
      virtual Widget* _process_unhandled_editor_input(const InputEvent&){return nullptr;} //pass input to children if they want it and then process it for ourselves if necessary ONLY FOR EDITOR RELATED THINGS (grab handles mostly)
      InputFilter _inputFilter = InputFilter::INPUT_FILTER_PASS_AND_PROCESS;
      virtual void _on_mouse_enter(){};
      virtual void _on_mouse_exit(){};
      virtual void _on_modality_gained(){}
      virtual void _on_modality_lost(){}
      virtual void _on_focus_gained(){}
      virtual void _on_focus_lost(){}
      bool acceptsHover = false;
      bool isLayout = false;
      bool enabled = true; //changes visuals and (typically) ingores input

      std::shared_ptr<Theme> theme;

      void __on_rect_changed(const Rect<R_FLOAT>& oldRect, bool byLayout = false) override {
         if (!byLayout) {
            //layout will have already resized children
            calculateAnchoring(getRect());
         }
         _on_rect_changed();
         auto event = RectChangedEvent(this, oldRect);
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
         if (auto parent = getParentWidget()) {
            if (parent) {
               parent.value()->_on_child_rect_changed(this);
            }
         }
      }
      void calculateAnchoring(const Rect<R_FLOAT>& oldRect);
      virtual void _on_child_rect_changed(Widget*){};
      virtual void _on_rect_changed(){};
      Anchor _anchor = Anchor::NONE;

   private:
      void __init() override;
      void __on_added_to_tree() override;
      Widget* _parentWidget = nullptr; //the closest related parent that is a widget.
      bool _modal = false;

   #ifndef NDEBUG
      // Debug build
      static constexpr bool isDebugBuild = true;
      std::string _name = "nobody";
   #else
      static constexpr bool isDebugBuild = false;
   #endif

      friend class Layout;
      friend class Canvas;

public:
   //convenience forwards
   template <typename... Args>
   auto addChild(Args&& ...args){
      if (auto hasNode = getNode()) {
         return getNode()->addChild(std::forward<Args>(args)...);
      }
      throw std::runtime_error("Invalid Node for Widget::addChild");
   }
      template <typename... Args>
      auto removeChild(Args&& ...args){
         if (auto hasNode = getNode()) {
            return getNode()->removeChild(std::forward<Args>(args)...);
         }
         throw std::runtime_error("Invalid Node for Widget::removeChild");
      }
      template <typename... Args>
      auto removeAllChildren(Args&& ...args){
         if (auto hasNode = getNode()) {
            return getNode()->removeAllChildren(std::forward<Args>(args)...);
         }
         throw std::runtime_error("Invalid Node for Widget::removeChild");
      }
   };
}