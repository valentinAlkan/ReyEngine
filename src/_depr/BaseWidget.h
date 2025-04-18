#pragma once
#include "Property.h"
#include "TypeManager.h"
#include "InputManager.h"
#include "Theme.h"
#include "FileSystem.h"
#include "CollisionShape.h"
#include "Renderable2D.h"
#include "InputHandler.h"
#include <iostream>
#include <stack>
#include <utility>
#include <vector>
#include <memory>
#include <mutex>
#include <map>
#include <optional>
#include <unordered_set>
#include <unordered_map>
#include <variant>

#define DEBUG_FILL 1

namespace ReyEngine{
   class Window;
   class Scene;
   class Draggable;
   class Canvas;
   class Camera2D;
   class BaseWidget
   : public Internal::Renderable2D
   , public virtual Internal::Component
   , public virtual Internal::TypeContainer<BaseWidget>
   , public Internal::InputHandler
   {
      using ChildIndex = unsigned long;
      using WidgetPtr = std::shared_ptr<BaseWidget>;
      using ChildOrder = std::vector<std::shared_ptr<BaseWidget>>;
      static constexpr bool verbose = false;
   public:
      //disambiguation
      using Internal::TypeContainer<BaseWidget>::toEventSubscriber;
      using Internal::TypeContainer<BaseWidget>::toEventPublisher;
      using Internal::TypeContainer<BaseWidget>::publish;
//      using Internal::TypeContainer<BaseWidget>::getName;
//      using Internal::TypeContainer<BaseWidget>::getChild;
//      using Internal::TypeContainer<BaseWidget>::getChildMap;
//      using Internal::TypeContainer<BaseWidget>::getChildren;
//      using Internal::TypeContainer<BaseWidget>::getParent;
//      using Internal::TypeContainer<BaseWidget>::addChild;
//      using Internal::TypeContainer<BaseWidget>::free;
//      using Internal::TypeContainer<BaseWidget>::findDescendents;
//      using Internal::TypeContainer<BaseWidget>::removeChild;
//      using Internal::TypeContainer<BaseWidget>::removeAllChildren;
//      using Internal::TypeContainer<BaseWidget>::toContainedType;
//      using Internal::TypeContainer<BaseWidget>::toContainedTypePtr;
      using Internal::TypeContainer<BaseWidget>::ChildPtr;
      using Internal::TypeContainer<BaseWidget>::isRoot;
      using Internal::TypeContainer<BaseWidget>::setRoot;
      using Internal::TypeContainer<BaseWidget>::getScenePath;

      struct WidgetRectChangedEvent : public Event<WidgetRectChangedEvent> {
         EVENT_CTOR_SIMPLE(WidgetRectChangedEvent, Event<WidgetRectChangedEvent>){
            rect = publisher->toPublisherType<BaseWidget>()->getRect();
         }
         Rect<R_FLOAT> rect;
      };

      struct WidgetUnhandledInputEvent : public Event<WidgetUnhandledInputEvent> {
         EVENT_CTOR_SIMPLE(WidgetUnhandledInputEvent, Event<WidgetUnhandledInputEvent>, const InputEvent& fwdEvent, const std::optional<UnhandledMouseInput>& fwdMouse)
         , fwdEvent(fwdEvent)
         , fwdMouse(fwdMouse)
         {}
         Rect<R_FLOAT> rect;
         const InputEvent& fwdEvent;
         const std::optional<UnhandledMouseInput> fwdMouse;
         bool handled = false;
      };


      //Input masking controls whether input inside the rect is masked (ignored)
      enum InputMask {NONE, IGNORE_INSIDE, IGNORE_OUTSIDE};
      template <typename T>
      struct InputMaskProperty : public EnumProperty<InputMask, 3>{
           InputMaskProperty(const std::string& instanceName,  InputMask defaultvalue)
           : EnumProperty<InputMask, 3>(instanceName, std::move(defaultvalue))
           {}
           const EnumPair<InputMask, 3>& getDict() const  override {return dict;}
           static constexpr EnumPair<InputMask, 3> dict = {
                   ENUM_PAIR_DECLARE(InputMask, NONE),
                   ENUM_PAIR_DECLARE(InputMask, IGNORE_INSIDE),
                   ENUM_PAIR_DECLARE(InputMask, IGNORE_OUTSIDE),
           };
          InputMaskProperty(const InputMaskProperty& other){EnumProperty::value = other.value; mask = other.mask;}
          InputMaskProperty& operator=(const InputMaskProperty& other){EnumProperty::value = other.value; mask = other.mask; return *this;}
          Rect<T> mask;
           std::string toString() const override {return  EnumProperty::toString() + "," + mask.toString();}
           InputMask fromString(const std::string& str) override {
               auto split = string_tools::split(",");
               EnumProperty::fromString(split.at(0));
               mask = Rect<R_FLOAT>::fromString(split.at(1));
               return *this;
           }
       };

      enum class Anchor{NONE, LEFT, RIGHT, TOP, BOTTOM, FILL, TOP_LEFT, TOP_RIGHT, BOTTOM_RIGHT, BOTTOM_LEFT, CENTER};
      /////////////////////////////////////////////////////////////////////////////////////////
      struct AnchorProperty : public EnumProperty<Anchor, 11>{
         AnchorProperty(const std::string& instanceName,  Anchor defaultvalue)
         : EnumProperty<Anchor, 11>(instanceName, std::move(defaultvalue))
         {}
         const EnumPair<Anchor, 11>& getDict() const  override {return dict;}
         static constexpr EnumPair<Anchor, 11> dict = {
               ENUM_PAIR_DECLARE(Anchor, NONE),
               ENUM_PAIR_DECLARE(Anchor, LEFT),
               ENUM_PAIR_DECLARE(Anchor, RIGHT),
               ENUM_PAIR_DECLARE(Anchor, TOP),
               ENUM_PAIR_DECLARE(Anchor, BOTTOM),
               ENUM_PAIR_DECLARE(Anchor, BOTTOM_LEFT),
               ENUM_PAIR_DECLARE(Anchor, BOTTOM_RIGHT),
               ENUM_PAIR_DECLARE(Anchor, TOP_LEFT),
               ENUM_PAIR_DECLARE(Anchor, TOP_RIGHT),
               ENUM_PAIR_DECLARE(Anchor, FILL),
               ENUM_PAIR_DECLARE(Anchor, CENTER)
         };
      };

      struct WidgetProperty : public Property<WidgetPtr>{
         using Property<WidgetPtr>::operator=;
         WidgetProperty(const std::string& instanceName, WidgetPtr defaultvalue = nullptr)
         : Property(instanceName, PropertyTypes::BaseWidget, std::move(defaultvalue))
         {}
         std::string toString() const override {return value->serialize();}
         WidgetPtr fromString(const std::string& data) override { throw std::runtime_error("not implemented"); return {};}
      };
       /////////////////////////////////////////////////////////////////////////////////////////

      static constexpr char TYPE_NAME[] = "BaseWidget";
      BaseWidget(const std::string& name, std::string  typeName);
      ~BaseWidget() override;

      //rect stuff
      Rect<R_FLOAT> getChildBoundingBox() const; //get the smallest rectangle that contains all children, starting from 0,0. Does not include grandchildren.
      Size<R_FLOAT> getMinSize(){return minSize;}
      Size<R_FLOAT> getMaxSize(){return maxSize;}
      Size<R_FLOAT> getClampedSize();
      Size<R_FLOAT> getClampedSize(Size<R_FLOAT>);
      //sizing
      void setAnchoring(Anchor newAnchor);
      Anchor getAnchoring(){return _anchor.value;}
      bool isAnchored(){return _anchor.value != Anchor::NONE;}
      void setMaxSize(const Size<R_FLOAT>& size);
      void setMaxWidth(R_FLOAT maxWidth){maxSize.x = maxWidth;}
      void setMaxHeight(R_FLOAT maxHeight){maxSize.y = maxHeight;}
      void setMinSize(const Size<R_FLOAT>& size);
      void setMinWidth(R_FLOAT minWidth){minSize.x = minWidth;}
      void setMinHeight(R_FLOAT minHeight){minSize.y = minHeight;}
      void setRect(const Rect<R_FLOAT>& r);
      void setPos(R_FLOAT x, R_FLOAT y);
      void setPos(const Pos<R_FLOAT>& pos);
      void setPosRelative(const Pos<R_FLOAT>& pos, const Pos<R_FLOAT>& basis); //sets a new position relative to its current position
      void setX(R_FLOAT x);
      void setY(R_FLOAT y);
      void move(const Pos<R_FLOAT>& amt);
      void setSize(const Size<R_FLOAT>& size);
      void scale(const Vec2<float>& scale);
      void setWidth(R_FLOAT width);
      void setHeight(R_FLOAT height);

      Pos<R_FLOAT> getLocalMousePos() const {return getCanvasInputPos().get() - getGlobalPos().get();}
      bool isInside(const Pos<R_FLOAT>& point) const {return getRect().toSizeRect().isInside(point);}
      bool setName(const std::string& name, bool append_index=false);
      bool setIndex(unsigned int newIndex);
      std::string getTypeName() const {return _typeName;}

      std::optional<std::shared_ptr<BaseWidget>> getWidgetAt(Pos<R_FLOAT> pos);
      bool isHovered() const {return _hovered;}
      void setInputFilter(InputFilter newFilter){ _inputFilter = newFilter;}
      InputFilter getInputFilter(){return _inputFilter;}

      WidgetPtr toBaseWidget(){return toType<BaseWidget>();}

      void setAcceptsHover(bool accepts){acceptsHover = accepts;} //only way to get mouse_enter and mouse_exit
      bool getAcceptsHover() const {return acceptsHover;}
      void setProcess(bool process);
      void setBackRender(bool);

      template <typename T> bool is_base_of(){return std::is_base_of_v<BaseWidget, T>;}
      inline std::shared_ptr<Style::Theme>& getTheme(){return theme;}
      inline const std::shared_ptr<Style::Theme>& getTheme() const {return theme;}
      inline void setTheme(std::shared_ptr<Style::Theme>& newTheme){theme = newTheme;}

      //misc
      void setEnabled(bool newEnabled){ enabled.value = newEnabled;}
      bool getEnabled(){ return enabled.value;}
   protected:

      void __on_rect_changed(const Rect<R_FLOAT>& oldRect); //internal. Trigger resize for anchored widgets.
      void __on_child_removed(ChildPtr&) override;
      void __on_exit_tree() override {}
      void __on_enter_tree() override;
      void __on_added_to_parent() override;

      void _on_child_removed(ChildPtr& child) override {
         child->isInLayout = false;
      }
      void __on_child_added_immediate(ChildPtr&);
      void __on_child_added_immediate_basewidget(ChildPtr&);
      virtual std::optional<std::shared_ptr<Draggable>> _on_drag_start(Pos<R_FLOAT> globalPos){return std::nullopt;} //override and return something to implement drag and drop
      virtual Handled _on_drag_drop(std::shared_ptr<Draggable>){return false;}

      //override and setProcess(true) to allow processing
      virtual void _process(float dt){};
      void registerProperties() override;
      InputMaskProperty<R_FLOAT> _inputMask; //Only input inside this rectangle will be handled;
      BoolProperty enabled;

   public:
      //input
      virtual Handled _unhandled_input(const InputEvent&, const std::optional<UnhandledMouseInput>&){return false;}
      virtual Handled _unhandled_masked_input(const InputEventMouse&, const std::optional<UnhandledMouseInput>&){return true;} //masked input is handled by default
      virtual std::optional<std::shared_ptr<BaseWidget>> askHover(const Pos<R_FLOAT>& globalPos);
   protected:
      void _is_extendable(){static_assert(true);}
      virtual std::string _ge_t_static_constexpr_typename(){return TYPE_NAME;}

      //convenience
      void _publishSize(){WidgetRectChangedEvent event(toEventPublisher()); publish<WidgetRectChangedEvent>(event);}

      bool isLayout = false;
      bool isInLayout = false;
      bool acceptsHover = false;
      Size<R_FLOAT> maxSize = {MaxFloat, MaxFloat};
      Size<R_FLOAT> minSize = {0, 0};
      BoolProperty isBackRender;

   public:
      //modality - modal widgets should be children of the root canvas, otherwise they will
      void setModal(bool isModal);
      bool isModal() const {return _isModal;};
      std::optional<std::shared_ptr<Canvas>> getCanvas(); //get the most closely-related parent canvas this widget belongs to

      //focus - first crack at input
      bool isFocus(){return _isFocus;}
      void setFocus(bool isFocus);

      //editor stuff
      inline void setInEditor(bool state){_isEditorWidget = state;}
      inline bool isInEditor() const {return _isEditorWidget;}
      inline void setEditorSelected(bool selected){_editor_selected = selected;}
      inline bool isEditorSelected() const {return _editor_selected;}
   protected:
      bool _isCamera = false;
      bool _isEditorWidget = false; //true if this is a widget THE USER HAS PLACED IN THE EDITOR WORKSPACE (not a widget that the editor uses for normal stuff)
      bool _editor_selected = false; // true when the object is *selected* in the editor
      static constexpr int GRAB_HANDLE_SIZE = 10;
      Rect<R_FLOAT> _getGrabHandle(int index);// 0-3 clockwise starting in top left (TL,TR,BR,BL)
      int _editor_grab_handles_dragging = -1; //which grab handle is being drug around
      std::vector<BaseWidget*> _frontRenderList; //children to be rendered IN FRONT of this widget (normal behavior)
      std::vector<BaseWidget*> _backRenderList; //children to be rendered BEHIND this widget

      AnchorProperty _anchor;
      void rename(WidgetPtr& child, const std::string& newName);
      ///If this widget is the root of a scene, then the rest of the scene data is here.
      std::optional<std::shared_ptr<Scene>> _scene;
      bool _request_delete = false; //true when we want to remove this object from the tree
      bool _hovered = false; //true when hovered, set by application
      bool _isModal = false;
      bool _isFocus = false;
      bool _scheduled_for_deletion = false; // true when the widget has been scheduled for deletion but is not yet deleted.
//      Pos<R_FLOAT> _renderOffset; //used for different rendering modes. does not offset position.
      InputInterface::MouseCursor cursor = InputInterface::MouseCursor::DEFAULT;

      //input
      Handled _process_unhandled_input(const InputEvent&, const std::optional<UnhandledMouseInput>&); //pass input to children if they want it and then process it for ourselves if necessary
      virtual Handled __process_unhandled_input(const InputEvent& event, const std::optional<UnhandledMouseInput>& mouse){ return _process_unhandled_input(event, mouse);}
      Handled _process_unhandled_editor_input(const InputEvent&, const std::optional<UnhandledMouseInput>&); //pass input to children if they want it and then process it for ourselves if necessary ONLY FOR EDITOR RELATED THINGS (grab handles mostly)
      UnhandledMouseInput toMouseInput(const CanvasSpace<Pos<R_FLOAT>>& canvas) const;
      InputFilter _inputFilter = InputFilter::INPUT_FILTER_PASS_AND_PROCESS;

      //theme
      friend class Window;
      friend class Canvas;
      friend class Application;
      friend class ScrollArea;
      friend class Camera2D;
      friend class Internal::TypeContainer<BaseWidget>;
   };
}