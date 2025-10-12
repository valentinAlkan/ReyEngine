#pragma once
#include "Drawable2D.h"
#include "InputHandler.h"
#include "Processable.h"
#include "Theme.h"
#include "WeakUnits.h"
#include "MetaData.h"

namespace ReyEngine {
   enum class Anchor{NONE, LEFT, RIGHT, TOP, TOP_WIDTH, BOTTOM, FILL, TOP_LEFT, TOP_RIGHT, BOTTOM_RIGHT, BOTTOM_LEFT, CENTER, CUSTOM};
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

      EVENT_ARGS(EventAnchoring, 982347093, const Rect<float>& oldRect, Rect<float>& newRect, Widget* parentWidget)
      , newRect(newRect)
      , oldRect(oldRect)
      , parentWidget(parentWidget)
         {}
         Rect<float>& newRect;
         const Rect<float>& oldRect;
         Widget* parentWidget;
      };

      EVENT_ARGS(WidgetUnhandledInputEvent, 329875, const InputEvent& fwdEvent)
         , fwdEvent(fwdEvent)
         {}
         Rect<R_FLOAT> rect;
         const InputEvent& fwdEvent;
         Widget* handler = nullptr;
      };

      Widget()= default;
      ~Widget() override{
         if constexpr (isDebugBuild) {
            std::cout << "Goodbye from " << getName() << "!!" << std::endl;
         }
      }
      REYENGINE_OBJECT(Widget)
      Theme& getTheme(){return *theme;}
      const Theme& getTheme() const {return *theme;}
      void setTheme(std::shared_ptr<Theme>& newTheme){theme = newTheme;}
      void setTheme(std::shared_ptr<Theme>&& newTheme){theme = std::move(newTheme);}
      [[nodiscard]] std::optional<Widget*> getParentWidget() const;
      void setAnchoring(Anchor newAnchor);
      [[nodiscard]] Rect<R_FLOAT> getChildBoundingBox() const;
      [[nodiscard]] Anchor getAnchoring() const {return _anchor;}
      [[nodiscard]] FrameCount getEngineFrameCount() const;
      [[nodiscard]] bool isHovered() const;
      [[nodiscard]] bool isFocused() const;
      [[nodiscard]] bool isModal() const;
      void setHovered(bool);
      void setFocused(bool);
      void setModal(bool);
      [[nodiscard]] Pos<float> getLocalMousePos() const;
      CanvasSpace<Pos<float>> toCanvasSpace(const Pos<float>&);
      WindowSpace<Pos<float>> toWindowSpace(const Pos<float>&);
      void setEnabled(bool newState){enabled = newState;}
      [[nodiscard]] bool getIsEnabled() const {return enabled;}
      void setInputFiltering(InputFilter newFilter){ _inputFilter = newFilter;}
      [[nodiscard]] InputFilter getInputFiltering() const {return _inputFilter;}
      void setAcceptsHover(bool accepts);
      //convenience
      void moveToForeground();
      void moveToBackground();
   protected:
      //input
      virtual Widget* __process_unhandled_input(const InputEvent& event){ return _unhandled_input(event);}
      virtual Widget* _unhandled_input(const InputEvent&){return nullptr;}
      virtual Widget* _process_unhandled_editor_input(const InputEvent&){return nullptr;} //pass input to children if they want it and then process it for ourselves if necessary ONLY FOR EDITOR RELATED THINGS (grab handles mostly)
      virtual void _on_mouse_enter(){};
      virtual void _on_mouse_exit(){};
      virtual void _on_modality_gained(){}
      virtual void _on_modality_lost(){}
      virtual void _on_focus_gained(){}
      virtual void _on_focus_lost(){}
      virtual void _on_visibility_changed(){}
      virtual void _on_child_rect_changed(Widget*){};
      virtual void _on_rect_changed(){};

      bool acceptsHover = false;
      bool isLayout = false;
      bool enabled = true; //changes visuals and (typically) ingores input
      InputFilter _inputFilter = InputFilter::PASS_AND_PROCESS;
      Anchor _anchor = Anchor::NONE;

      std::shared_ptr<Theme> theme;
   private:
      void __init() override;
      void __on_added_to_tree() override;
      void __on_visibility_changed() override {
         if (!_visible && getCanvas()) {
            //strip widgets of focus and modality when they are no longer visible. Otherwise we can softlock.
            setFocused(false);
            setModal(false);
         }
         _on_visibility_changed();
      }

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

      Widget* _parentWidget = nullptr; //the closest related parent that is a widget.
      bool _modal = false;

   #ifndef NDEBUG
      // Debug build
      static constexpr bool isDebugBuild = true;
//      std::string _name = "nobody";
   #else
      static constexpr bool isDebugBuild = false;
   #endif

      friend class Layout;
      friend class Canvas;

      [[noreturn]] void throwEx(auto widget, auto operation) {
         throw std::runtime_error(std::string("Widget failed operation \"") + operation + "\" on " + widget->getName() + " : " + std::string(INVALID_NODE_ERR_MSG));
      }

   public:
      //convenience forwards
      template <typename... Args>
      auto addChild(Args&& ...args){
         if (auto hasNode = getNode()) {
            return getNode()->addChild(std::forward<Args>(args)...);
         }
         throwEx(std::get<0>(std::forward_as_tuple(std::forward<Args>(args)...)), "addChild");
      }
      template <typename... Args>
      auto removeChild(Args&& ...args){
         if (auto hasNode = getNode()) {
            return getNode()->removeChild(std::forward<Args>(args)...);
         }
         throwEx(std::get<0>(std::forward_as_tuple(std::forward<Args>(args)...)), "removeChild");
      }

      auto removeAllChildren(){
         if (auto hasNode = getNode()) {
            return hasNode->removeAllChildren();
         }
         throwEx(this, "removeAllChildren");
      }
   private:
      static constexpr std::string_view INVALID_NODE_ERR_MSG = "Invalid Node for operation. \n"
                                                               "Did you call this from a Widget constructor? That doesn't work since the associated node hasn't been built yet.\n"
                                                               "Override _on_made() or _init() to ensure that the node associated with an object has been fully built.\n"
                                                               "_on_made() occurs when make_node is called, _init() is called when the node is first added to the tree.\n"
                                                               "_init() should be your first choice unless you need to do things at build time, as the node will know who \n"
                                                               "its parent is during _init()";
   };
}