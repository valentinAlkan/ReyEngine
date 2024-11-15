#pragma once
#include "raylib.h"
#include "DrawInterface.h"
#include "Property.h"
#include "TypeManager.h"
#include "InputManager.h"
#include "EventManager.h"
#include "Theme.h"
#include "Component.h"
#include "FileSystem.h"
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

using Handled = bool;

namespace ReyEngine{

   class Scene;
   class Draggable;
   class Canvas;
   class BaseWidget
   : public virtual Internal::Component
   , public Internal::TypeContainer<BaseWidget>
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
      using Internal::TypeContainer<BaseWidget>::getName;
      using Internal::TypeContainer<BaseWidget>::getChild;
      using Internal::TypeContainer<BaseWidget>::getChildMap;
      using Internal::TypeContainer<BaseWidget>::getChildren;
      using Internal::TypeContainer<BaseWidget>::getParent;
      using Internal::TypeContainer<BaseWidget>::addChild;
      using Internal::TypeContainer<BaseWidget>::removeChild;
      using Internal::TypeContainer<BaseWidget>::removeAllChildren;
      using Internal::TypeContainer<BaseWidget>::toContainedType;
      using Internal::TypeContainer<BaseWidget>::toContainedTypePtr;
      using Internal::TypeContainer<BaseWidget>::ChildPtr;
      using Internal::TypeContainer<BaseWidget>::isRoot;
      using Internal::TypeContainer<BaseWidget>::setRoot;
      using Internal::TypeContainer<BaseWidget>::getScenePath;

      struct WidgetRectChangedEvent : public Event<WidgetRectChangedEvent> {
         EVENT_CTOR_SIMPLE(WidgetRectChangedEvent, Event<WidgetRectChangedEvent>){
            rect = publisher->toPublisherType<BaseWidget>()->getRect();
         }
         ReyEngine::Rect<R_FLOAT> rect;
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
          ReyEngine::Rect<T> mask;
           std::string toString() const override {return  EnumProperty::toString() + "," + mask.toString();}
           InputMask fromString(const std::string& str) override {
               auto split = string_tools::split(",");
               EnumProperty::fromString(split.at(0));
               mask = ReyEngine::Rect<R_FLOAT>::fromString(split.at(1));
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
      ReyEngine::Rect<R_FLOAT> getRect() const {return _rect.value;}
      ReyEngine::Rect<R_FLOAT> getGlobalRect() const {auto globalPos = getGlobalPos(); return {globalPos.x, globalPos.y, getSize().x, getSize().y};}
      ReyEngine::Pos<R_FLOAT> getGlobalPos() const;
      ReyEngine::Size<R_FLOAT> getChildBoundingBox() const; //get the smallest rectangle that contains all children, starting from 0,0. Does not include grandchildren.
      ReyEngine::Pos<R_FLOAT> getPos() const {return {_rect.value.x, _rect.value.y};}
      ReyEngine::Size<R_FLOAT> getSize() const {return getRect().size();}
      R_FLOAT getWidth() const {return _rect.value.width;}
      R_FLOAT getHeight() const {return _rect.value.height;}
      Vec2<R_FLOAT> getHeightRange() const {return {0, getRect().size().y};}
      Vec2<R_FLOAT> getWidthtRange() const {return {0, getRect().size().x};}
      ReyEngine::Size<R_FLOAT> getMinSize(){return minSize;}
      ReyEngine::Size<R_FLOAT> getMaxSize(){return maxSize;}
      ReyEngine::Size<R_FLOAT> getClampedSize();
      ReyEngine::Size<R_FLOAT> getClampedSize(ReyEngine::Size<R_FLOAT>);
      void setVisible(bool visible){_visible = visible;}
      bool getVisible() const {return _visible;}
      //sizing
      void setAnchoring(Anchor newAnchor);
      Anchor getAnchoring(){return _anchor.value;}
      bool isAnchored(){return _anchor.value != Anchor::NONE;}
      void setMaxSize(const ReyEngine::Size<R_FLOAT>& size);
      void setMaxWidth(int maxWidth){maxSize.x = maxWidth;}
      void setMaxHeight(int maxHeight){maxSize.y = maxHeight;}
      void setMinSize(const ReyEngine::Size<R_FLOAT>& size);
      void setMinWidth(int minWidth){minSize.x = minWidth;}
      void setMinHeight(int minHeight){minSize.y = minHeight;}
      void setRect(const ReyEngine::Rect<R_FLOAT>& r);
      void setPos(int x, int y);
      void setPos(const ReyEngine::Pos<R_FLOAT>& pos);
      void setPosRelative(const ReyEngine::Pos<R_FLOAT>& pos, const ReyEngine::Pos<R_FLOAT>& basis); //sets a new position relative to its current position
      void setX(int x);
      void setY(int y);
      void move(const ReyEngine::Pos<R_FLOAT>& amt);
      void setSize(const ReyEngine::Size<R_FLOAT>& size);
      void scale(const ReyEngine::Vec2<float>& scale);
      void setWidth(int width);
      void setHeight(int height);

      ReyEngine::Pos<R_FLOAT> getLocalMousePos() const {return globalToLocal(InputManager::getMousePos());}
      ReyEngine::Pos<R_FLOAT> globalToLocal(const ReyEngine::Pos<R_FLOAT>& global) const;
      ReyEngine::Pos<R_FLOAT> localToGlobal(const ReyEngine::Pos<R_FLOAT>& local) const;
      ReyEngine::Rect<R_FLOAT> globalToLocal(const ReyEngine::Rect<R_FLOAT>& global) const;
      ReyEngine::Rect<R_FLOAT> localToGlobal(const ReyEngine::Rect<R_FLOAT>& local) const;
      void setGlobalPos(const Vec2<R_FLOAT>&);
      bool isInside(const Vec2<R_FLOAT>& point) const {return _rect.value.toSizeRect().isInside(point);}
      bool setName(const std::string& name, bool append_index=false);
      bool setIndex(unsigned int newIndex);
      std::string getTypeName() const {return _typeName;}

      std::optional<std::shared_ptr<BaseWidget>> getWidgetAt(ReyEngine::Pos<R_FLOAT> pos);
//      std::weak_ptr<BaseWidget> getParent(){return _parent;}
//      const ChildOrder& getChildren() const {return _childrenOrdered;}
//      std::optional<WidgetPtr> getChild(const std::string& name);
//      std::vector<std::weak_ptr<BaseWidget>> findChild(const std::string& name, bool exact=false);
      bool isHovered() const {return _hovered;}
      void setInputFilter(InputFilter newFilter){ _inputFilter = newFilter;}
      InputFilter getInputFilter(){return _inputFilter;}

      template <typename T>
      std::shared_ptr<T> toType(){
         static_assert(std::is_base_of_v<BaseWidget, T>);
         return std::static_pointer_cast<T>(toContainedTypePtr());
      }
      WidgetPtr toBaseWidget(){return toType<BaseWidget>();}

      void setAcceptsHover(bool accepts){acceptsHover = accepts;} //only way to get mouse_enter and mouse_exit
      bool getAcceptsHover() const {return acceptsHover;}
      void setProcess(bool process);
//      WidgetPtr setFree(); //request to remove this widget from the tree at next available opportunity. Does not immediately delete it
                      // if you need your object to stop being processed immediately, use setFreeImmediately(), which could pause to syncrhonize threads.
//      WidgetPtr setFreeImmediately(); // Pauses other threads and immediately removes objects from the tree.

      virtual void render() const = 0; //draw the widget
      void setBackRender(bool);

      template <typename T> bool is_base_of(){return std::is_base_of_v<BaseWidget, T>;}
      inline std::shared_ptr<Style::Theme>& getTheme(){return theme;}
      inline void setTheme(std::shared_ptr<Style::Theme>& newTheme){theme = newTheme;}

      //drawing functions
      void drawLine(const ReyEngine::Line<R_FLOAT>&, float lineThick, const ReyEngine::ColorRGBA&) const;
      void drawArrow(const ReyEngine::Line<R_FLOAT>&, float lineThick, const ReyEngine::ColorRGBA&, float headSize=20) const;
      void drawText(const std::string& text, const ReyEngine::Pos<R_FLOAT>& pos, const ReyEngine::ReyEngineFont& font) const;
      void drawTextCentered(const std::string& text, const ReyEngine::Pos<R_FLOAT>& pos, const ReyEngine::ReyEngineFont& font) const;
      void drawRectangle(const ReyEngine::Rect<R_FLOAT>& rect, const ReyEngine::ColorRGBA& color) const;
      void drawRectangleLines(const ReyEngine::Rect<R_FLOAT>& rect, float lineThick, const ReyEngine::ColorRGBA& color) const;
      void drawRectangleRounded(const ReyEngine::Rect<R_FLOAT>& rect,  float roundness, int segments, const ReyEngine::ColorRGBA& color) const;
      void drawRectangleRoundedLines(const ReyEngine::Rect<R_FLOAT>& rect, float roundness, int segments, float lineThick, const ReyEngine::ColorRGBA& color) const;
      void drawRectangleGradientV(const ReyEngine::Rect<R_FLOAT>& rect, const ReyEngine::ColorRGBA& color1, const ReyEngine::ColorRGBA& color2) const;
      void drawCircle(const ReyEngine::Circle&, const ReyEngine::ColorRGBA&) const;
      void drawCircleLines(const ReyEngine::Circle&, const ReyEngine::ColorRGBA&) const;
      void drawCircleSectorLines(const ReyEngine::CircleSector&, const ReyEngine::ColorRGBA&, int segments) const;
      void drawRenderTarget(const ReyEngine::RenderTarget&, const ReyEngine::Pos<R_FLOAT>&) const;
      void drawRenderTargetRect(const ReyEngine::RenderTarget&, const ReyEngine::Rect<R_FLOAT>&, const ReyEngine::Pos<R_FLOAT>&) const;
      void drawTextureRect(const ReyEngine::ReyTexture&, const ReyEngine::Rect<R_FLOAT>& src, const ReyEngine::Rect<R_FLOAT>& dst, float rotation, const ReyEngine::ColorRGBA& tint) const;
      void startScissor(const ReyEngine::Rect<R_FLOAT>&) const;
      void stopScissor() const;
   protected:

      void __on_rect_changed(); //internal. Trigger resize for anchored widgets.
      void __on_child_removed(ChildPtr&) override;
      void __on_exit_tree() override {}
      void __on_enter_tree() override;
      virtual void _on_rect_changed(){} //called when the rect is manipulated
      virtual void _on_mouse_enter(){};
      virtual void _on_mouse_exit(){};
      virtual void _on_modality_gained(){}
      virtual void _on_modality_lost(){}
      void _on_child_removed(ChildPtr& child) override {
         child->isInLayout = false;
      }
      void __on_child_added_immediate(ChildPtr&);
      virtual std::optional<std::shared_ptr<Draggable>> _on_drag_start(ReyEngine::Pos<R_FLOAT> globalPos){return std::nullopt;} //override and return something to implement drag and drop
      virtual Handled _on_drag_drop(std::shared_ptr<Draggable>){return false;}

      //override and setProcess(true) to allow processing
      virtual void _process(float dt){};

      // Drawing functions
      virtual void renderBegin(ReyEngine::Pos<R_FLOAT>& textureOffset){}
      virtual void renderEnd(){}

      virtual void renderEditorFeatures();
      virtual void renderChain(ReyEngine::Pos<R_FLOAT>& textureOffset);
      ReyEngine::Vec2<float> getRenderOffset() const {return _renderOffset;}
      void setRenderOffset(ReyEngine::Pos<R_FLOAT> offset){_renderOffset = offset;}
   //   void renderTextureOffsetApply(ReyEngine::Pos<float>& textureOffset){}
   //   void renderTextureOffsetReset(ReyEngine::Pos<float>& textureOffset){}
      void registerProperties() override;
      RectProperty<R_FLOAT> _rect;
      InputMaskProperty<R_FLOAT> _inputMask; //Only input inside this rectangle will be handled;

      //input
   public:
      virtual Handled _unhandled_input(const InputEvent&, const std::optional<UnhandledMouseInput>&){return false;}
      virtual Handled _unhandled_masked_input(const InputEventMouse&, const std::optional<UnhandledMouseInput>&){return true;} //masked input is handled by default
      virtual std::optional<std::shared_ptr<BaseWidget>> askHover(const ReyEngine::Pos<R_FLOAT>& globalPos);
   protected:
      void _is_extendable(){static_assert(true);}
      virtual std::string _get_static_constexpr_typename(){return TYPE_NAME;}

      //convenience
      void _publishSize(){WidgetRectChangedEvent event(toEventPublisher()); publish<WidgetRectChangedEvent>(event);}

      bool isLayout = false;
      bool isInLayout = false;
      bool acceptsHover = false;
      ReyEngine::Size<R_FLOAT> maxSize = {ReyEngine::MaxFloat, ReyEngine::MaxFloat};
      ReyEngine::Size<R_FLOAT> minSize = {0, 0};
      BoolProperty isBackRender;

   public:
      //modality
      void setModal(bool isModal);
      bool isModal() const {return _isModal;};
      std::optional<std::shared_ptr<Canvas>> getCanvas(); //get the most closely-related parent canvas this widget belongs to

      //editor stuff
      inline void setInEditor(bool state){_isEditorWidget = state;}
      inline bool isInEditor() const {return _isEditorWidget;}
      inline void setEditorSelected(bool selected){_editor_selected = selected;}
      inline bool isEditorSelected() const {return _editor_selected;}
   protected:
      bool _isEditorWidget = false; //true if this is a widget THE USER HAS PLACED IN THE EDITOR WORKSPACE (not a widget that the editor uses for normal stuff)
      bool _editor_selected = false; // true when the object is *selected* in the editor
      static constexpr int GRAB_HANDLE_SIZE = 10;
      ReyEngine::Rect<R_FLOAT> _getGrabHandle(int index);// 0-3 clockwise starting in top left (TL,TR,BR,BL)
      int _editor_grab_handles_dragging = -1; //which grab handle is being drug around
      ChildOrder _frontRenderList; //children to be rendered IN FRONT of this widget (normal behavior)
      ChildOrder _backRenderList; //children to be rendered BEHIND this widget

      AnchorProperty _anchor;
      void rename(WidgetPtr& child, const std::string& newName);
      ///If this widget is the root of a scene, then the rest of the scene data is here.
      std::optional<std::shared_ptr<Scene>> _scene;
      bool _request_delete = false; //true when we want to remove this object from the tree
      bool _hovered = false; //true when hovered, set by application
      bool _visible = true; //whether to show the widget (and its children)
      bool _isModal = false;
      bool _scheduled_for_deletion = false; // true when the widget has been scheduled for deletion but is not yet deleted.
      ReyEngine::Pos<R_FLOAT> _renderOffset; //used for different rendering modes. does not offset position.
      InputInterface::MouseCursor cursor = InputInterface::MouseCursor::DEFAULT;

      //input
      Handled _process_unhandled_input(const InputEvent&, const std::optional<UnhandledMouseInput>&); //pass input to children if they want it and then process it for ourselves if necessary
      Handled _process_unhandled_editor_input(const InputEvent&, const std::optional<UnhandledMouseInput>&); //pass input to children if they want it and then process it for ourselves if necessary ONLY FOR EDITOR RELATED THINGS (grab handles mostly)
      UnhandledMouseInput toMouseInput(const ReyEngine::Pos<R_FLOAT>& global) const;
      InputFilter _inputFilter = InputFilter::INPUT_FILTER_PASS_AND_PROCESS;

      //theme
      std::shared_ptr<Style::Theme> theme;
      friend class Window;
      friend class Canvas;
      friend class Application;
      friend class ScrollArea;
   };
}