#pragma once
#include "Drawable2D.h"
#include "InputManager.h"
#include "Theme.h"
#include "WeakUnits.h"
#include "MetaData.h"

namespace ReyEngine {
   enum class Anchor{NONE, LEFT, RIGHT, TOP, TOP_WIDTH, BOTTOM, FILL, TOP_LEFT, TOP_RIGHT, BOTTOM_RIGHT, BOTTOM_LEFT, CENTER, CUSTOM};
   class FocusGroup;
   class Widget;
   class Widget
   : public Internal::Drawable2D
   , public Internal::Tree::Processable
   , public EventPublisher
   , public EventSubscriber
   , public MetaDataInterface
   , public Easable
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
         Handled handled;
      };

      Widget()= default;
      ~Widget() override{}
      REYENGINE_OBJECT(Widget)
      Theme& getTheme(){return *theme;}
      const Theme& getTheme() const {return *theme;}
      void setTheme(std::shared_ptr<Theme>& newTheme){theme = newTheme;}
      void setTheme(std::shared_ptr<Theme>&& newTheme){theme = std::move(newTheme);}
      [[nodiscard]] std::optional<Widget*> getParentWidget() const;
      void setAnchoring(Anchor newAnchor);
      void setAnchorArea(const Rect<float>& r){_anchorArea = r;}
      [[nodiscard]] std::optional<Rect<float>> getAnchorArea() const {return _anchorArea;}
      [[nodiscard]] Rect<R_FLOAT> getChildBoundingBox() const;
      [[nodiscard]] Anchor getAnchoring() const {return _anchor;}
      [[nodiscard]] EngineFrameCount getEngineFrameCount() const;
      [[nodiscard]] bool isHovered() const;
      [[nodiscard]] bool isFocused() const;
      [[nodiscard]] bool isModal() const;
      [[nodiscard]] bool getAcceptsHover() const {return _acceptsHover;}
      void setAcceptsHover(bool accepts){_acceptsHover = accepts;}
      void setHovered(bool);
      void setFocused(bool);
      void setModal(bool);
      [[nodiscard]] Pos<float> getLocalMousePos() const;
      [[nodiscard]] CanvasSpace<Rect<float>> toCanvasRect() const;
      void fromCanvasRect(const CanvasSpace<Rect<float>>&);
      [[nodiscard]] CanvasSpace<Pos<float>> toCanvasSpace(const Pos<float>&) const;
      [[nodiscard]] WindowSpace<Pos<float>> toWindowSpace(const Pos<float>&) const;
      [[nodiscard]] Transform2D getWindowTransform() const;
      void setEnabled(bool newState){_enabled = newState;}
      [[nodiscard]] bool getIsEnabled() const {return _enabled;}
      void setInputFiltering(InputFilter newFilter){ _inputFilter = newFilter;}
      [[nodiscard]] InputFilter getInputFiltering() const {return _inputFilter;}
      [[nodiscard]] bool getIgnoreOutsideInput() const {return _ignoreOutsideInput;}
      void setIgnoreOutsideInput(bool newValue){ _ignoreOutsideInput = newValue;}
      virtual Handled processInput(const InputEvent& e); //process an input local to this widget
      //convenience
      void moveToForeground();
      void moveToBackground();
      [[nodiscard]] bool getIsRendering() const; //determines if this widget and all it's ancestors are being drawn - checks visibility of all ancestors
      void setToolTipText(const std::string& t){_tooltipText = t;}
      [[nodiscard]] std::string getToolTipText() const {return _tooltipText;}
      [[nodiscard]] bool isLayout() const {return _isLayout;}
   protected:
      virtual Handled _processInput(const InputEvent& e); //process an input local to this widget
      virtual void render2D(RenderContext&) const {};
      Handled __process_unhandled_input(const InputEvent& event);
      virtual Handled _unhandled_input(const InputEvent&){return nullptr;}
      virtual Handled _process_unhandled_editor_input(const InputEvent&){return nullptr;} //pass input to children if they want it and then process it for ourselves if necessary ONLY FOR EDITOR RELATED THINGS (grab handles mostly)
      virtual void _on_mouse_enter(){};
      virtual void _on_mouse_exit(){};
      virtual void _on_modality_gained(){}
      virtual void _on_modality_lost(){}
      virtual void _on_focus_gained(){}
      virtual void _on_focus_lost(){}
      virtual void _on_visibility_changed(){}
      virtual void _on_child_rect_changed(Widget*){};
      virtual void _on_rect_changed(){};
      void __on_added_to_tree() override;

      bool _isLayout = false;
      bool _enabled = true; //changes visuals and (typically) ingores input
      InputFilter _inputFilter = InputFilter::PASS_AND_PROCESS;
      std::optional<Rect<float>> _anchorArea; //optional area to anchor to, if not our size rect
      Anchor _anchor = Anchor::NONE;
      bool _ignoreOutsideInput = true;
      bool _handleAllModalInput = true; //by default, modal widgets handle all input.
                                     // But this can be disabled, as in the case of drop down menus, which
                                     // want to offer input to the menu bar, but need to be drawn modally
       Widget* _parentWidget = nullptr; //the closest related parent that is a widget.
      bool _modal = false;
      std::string _tooltipText;
      bool _acceptsHover = true; //whether this will implicitly handle hover events
      std::unique_ptr<InputContext::Semaphore> _inputContext; //only care if the destructor fires
      std::shared_ptr<Theme> theme;
   private:
      void __init() override;
      void __on_visibility_changed() override {
         if (!_visible && getCanvas()) {
            //strip widgets of focus and modality when they are no longer visible. Otherwise we can softlock.
            setFocused(false);
            setModal(false);
         }
         _on_visibility_changed();
      }
      void __on_rect_changed(const Rect<R_FLOAT>& oldRect, const Rect<R_FLOAT>& newRect, bool allowAnchor, bool byLayout = false) override {
         if (allowAnchor) {
            //layout will have already resized children by now
            calculateAnchoring(newRect);
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
         if (byLayout) return; //children of layouts don't always inform their parents when the child rect is changed
         if (auto parent = getParentWidget()) {
            if (parent) {
               parent.value()->_on_child_rect_changed(this);
            }
         }
      }
      void calculateAnchoring(const Rect<R_FLOAT>& oldRect);
      friend class Layout;
      friend class Canvas;

      [[noreturn]] void throwEx(const std::string& operation) {
         throw std::runtime_error(std::string("Widget failed operation \"") + operation + "\" on " + getName() + " : " + std::string(INVALID_NODE_ERR_MSG));
      }

   public:
      //convenience forwards
      template <typename... Args>
      auto addChild(Args&& ...args){
         if (auto hasNode = getNode()) {
            return getNode()->addChild(std::forward<Args>(args)...);
         }
         throwEx("addChild");
      }
//      template <typename... Args>
//      auto replaceChild(Args&& ...args){
//         if (auto hasNode = getNode()) {
//            return getNode()->replaceChild(std::forward<Args>(args)...);
//         }
//         throwEx(std::get<0>(std::forward_as_tuple(std::forward<Args>(args)...)), "addChild");
//      }
      template <typename... Args>
      auto removeChild(Args&& ...args){
         if (auto hasNode = getNode()) {
            return getNode()->removeChild(std::forward<Args>(args)...);
         }
         throwEx("removeChild");
      }

      auto removeAllChildren(){
         if (auto hasNode = getNode()) {
            return hasNode->removeAllChildren();
         }
         throwEx("removeAllChildren");
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