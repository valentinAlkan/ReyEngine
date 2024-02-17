#pragma once
#include "raylib.h"
#include "DrawInterface.h"
#include "Property.h"
#include "TypeManager.h"
#include "InputManager.h"
#include "EventManager.h"
#include "Theme.h"
#include "Component.h"
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

using Handled = bool;

class Scene;
class Draggable;
class  BaseWidget
: public inheritable_enable_shared_from_this<BaseWidget>
, public EventSubscriber
, public EventPublisher
, public Component
{
   using ChildIndex = unsigned long;
   using WidgetPtr = std::shared_ptr<BaseWidget>;
   using ChildMap = std::map<std::string, std::pair<ChildIndex, WidgetPtr>>;
   using ChildOrder = std::vector<std::shared_ptr<BaseWidget>>;
   using fVec = ReyEngine::Vec2<float>;
   using iVec = ReyEngine::Vec2<int>;
   using dVec = ReyEngine::Vec2<double>;
public:

   struct WidgetResizeEvent : public Event<WidgetResizeEvent> {
      EVENT_CTOR_SIMPLE(WidgetResizeEvent, Event<WidgetResizeEvent>){
         size = publisher->toBaseWidget()->getSize();
      }
      ReyEngine::Size<float> size;
   };

   enum class Anchor{NONE, LEFT, RIGHT, TOP, BOTTOM, FILL, TOP_LEFT, TOP_RIGHT, BOTTOM_RIGHT, BOTTOM_LEFT, CENTER};
   /////////////////////////////////////////////////////////////////////////////////////////
   struct AnchorProperty : public EnumProperty<Anchor, 11>{
      AnchorProperty(const std::string& instanceName,  Anchor defaultvalue)
      : EnumProperty<Anchor, 11>(instanceName, defaultvalue)
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

   static constexpr char TYPE_NAME[] = "BaseWidget";
   BaseWidget(const std::string& name, std::string  typeName);
   ~BaseWidget();
   //this will eventually move to component
   ReyEngine::ComponentPath getPath();

   //rect stuff
   ReyEngine::Rect<int> getRect() const {return _rect.value;}
   ReyEngine::Rect<int> getGlobalRect() const {auto globalPos = getGlobalPos(); return {globalPos.x, globalPos.y, getSize().x, getSize().y};}
   ReyEngine::Pos<int> getGlobalPos() const;
   ReyEngine::Size<int> getChildBoundingBox() const; //get the smallest rectangle that contains all children, starting from 0,0. Does not include grandchildren.
   ReyEngine::Pos<int> getPos() const {return {_rect.value.x, _rect.value.y};}
   ReyEngine::Size<int> getSize() const {return getRect().size();}
   int getWidth() const {return _rect.value.width;}
   int getHeight() const {return _rect.value.height;}
   iVec getHeightRange() const {return {0, getRect().size().y};}
   iVec getWidthtRange() const {return {0, getRect().size().x};}
   ReyEngine::Size<int> getMinSize(){return minSize;}
   ReyEngine::Size<int> getMaxSize(){return maxSize;}
   ReyEngine::Size<int> getClampedSize();
   ReyEngine::Size<int> getClampedSize(ReyEngine::Size<int>);
   void setVisible(bool visible){_visible = visible;}
   bool getVisible() const {return _visible;}
   //sizing
   void setAnchoring(Anchor newAnchor);
   Anchor getAnchoring(){return _anchor.value;}
   bool isAnchored(){return _anchor.value != Anchor::NONE;}
   void setMaxSize(const ReyEngine::Size<int>& size){maxSize = size;}
   void setMinSize(const ReyEngine::Size<int>& size){minSize = size;}
   void setRect(const ReyEngine::Rect<int>& r);
   void setPos(int x, int y);
   void setPos(const ReyEngine::Pos<int>& pos);
   void setSize(const ReyEngine::Size<int>& size);
   void scale(const ReyEngine::Vec2<float>& scale);
   void setWidth(int width);
   void setHeight(int height);

   ReyEngine::Pos<int> getLocalMousePos(){return globalToLocal(InputManager::getMousePos());}
   ReyEngine::Pos<int> globalToLocal(const ReyEngine::Pos<int>& global) const;
   ReyEngine::Pos<int> localToGlobal(const ReyEngine::Pos<int>& local) const;
   void setGlobalPos(const iVec&);
   bool isInside(const iVec& point){return _rect.value.toSizeRect().isInside(point);}
   bool setName(const std::string& name, bool append_index=false);
   bool setIndex(unsigned int newIndex);
   std::string getTypeName(){return _typeName;}

   std::optional<std::shared_ptr<BaseWidget>> getWidgetAt(ReyEngine::Pos<int> pos);
   std::weak_ptr<BaseWidget> getParent(){return _parent;}
   const ChildOrder& getChildren() const {return _childrenOrdered;}
   std::optional<WidgetPtr> getChild(const std::string& name);
   bool hasChild(const std::string& name);
   bool isHovered() const{return _hovered;}

   template <typename T>
   std::shared_ptr<T> toType(){
      static_assert(std::is_base_of_v<BaseWidget, T>);
      auto me = toBaseWidget();
      return std::static_pointer_cast<T>(me);
   }

   void setProcess(bool process);
   WidgetPtr setFree(); //request to remove this widget from the tree at next available opportunity. Does not immediately delete it
                   // if you need your object to stop being processed immediately, use setFreeImmediately(), which could pause to syncrhonize threads.
   WidgetPtr setFreeImmediately(); // Pauses other threads and immediately removes objects from the tree.

   virtual void render() const = 0; //draw the widget
   bool isRoot() const;

   std::optional<WidgetPtr> addChild(WidgetPtr);
   std::optional<WidgetPtr> removeChild(const std::string& name, bool quiet = false); //quiet silences the output if child is not found.
   void removeAllChildren(); //removes all children and DOES NOT RETURN THEM!


   template <typename T> bool is_base_of(){return std::is_base_of_v<BaseWidget, T>;}
   static void registerType(const std::string& typeName, const std::string& parentType, bool isVirtual, Deserializer fx){TypeManager::registerType(typeName, parentType, isVirtual, fx);}
   std::string serialize();
   std::shared_ptr<Style::Theme>& getTheme(){return theme;}
protected:
//   void recalculateRect();
   std::shared_ptr<BaseWidget> toBaseWidget(){return inheritable_enable_shared_from_this<BaseWidget>::downcasted_shared_from_this<BaseWidget>();}
   virtual void _on_application_ready(){}; //called when the main loop is starting, or immediately if that's already happened
   virtual void _init(){}; //run ONCE PER OBJECt when it enters tree for first time.
   void __on_rect_changed(); //internal. Trigger resize for anchored widgets.
   virtual void _on_rect_changed(){} //called when the rect is manipulated
   virtual void _on_mouse_enter(){};
   virtual void _on_mouse_exit(){};
   virtual void _on_child_added_immediate(WidgetPtr&){} //Called immediately upon a call to addChild - DANGER: widget is not actually a child yet! It is (probably) a very bad idea to do much at all here. Make sure you know what you're doing.
   void __on_child_added(WidgetPtr); //internal. Trigger resize for anchored widgets.
   virtual void _on_child_added(WidgetPtr&){} // called at the beginning of the next frame after a child is added. Child is now owned by us. Safe to manipulate child.
   virtual void _on_enter_tree(){} //called every time a widget enters the tree
   virtual void _on_exit_tree(){}
   virtual void _on_child_removed(){}
   virtual std::optional<std::shared_ptr<Draggable>> _on_drag_start(ReyEngine::Pos<int> globalPos){return std::nullopt;} //override and return something to implement drag and drop
   virtual Handled _on_drag_drop(std::shared_ptr<Draggable>){return false;}

   //override and setProcess(true) to allow processing
   virtual void _process(float dt){};

   // Drawing functions
   virtual void renderBegin(ReyEngine::Pos<double>& textureOffset){}
   void renderEditorFeatures();
   void renderChain(ReyEngine::Pos<double>& textureOffset);
   virtual void renderEnd(){}
   ReyEngine::Vec2<float> getRenderOffset() const {return _renderOffset;}
   void setRenderOffset(ReyEngine::Pos<double> offset){_renderOffset = offset;}
//   void renderTextureOffsetApply(ReyEngine::Pos<float>& textureOffset){}
//   void renderTextureOffsetReset(ReyEngine::Pos<float>& textureOffset){}
   void _drawText(const std::string& text, const ReyEngine::Pos<int>& pos, const ReyEngine::ReyEngineFont& font) const;
   void _drawTextCentered(const std::string& text, const ReyEngine::Pos<int>& pos, const ReyEngine::ReyEngineFont& font) const;
   void _drawRectangle(const ReyEngine::Rect<int>& rect, ReyEngine::ColorRGBA color) const;
   void _drawRectangleLines(const ReyEngine::Rect<int>& rect, float lineThick, ReyEngine::ColorRGBA color) const;
   void _drawRectangleRounded(const ReyEngine::Rect<int>& rect,  float roundness, int segments, ReyEngine::ColorRGBA color) const;
   void _drawRectangleRoundedLines(const ReyEngine::Rect<int>& rect, float roundness, int segments, float lineThick, ReyEngine::ColorRGBA color) const;
   void _drawRectangleGradientV(const ReyEngine::Rect<int>& rect, ReyEngine::ColorRGBA color1, ReyEngine::ColorRGBA color2) const;
   void registerProperties() override;
   void _deserialize(PropertyPrototypeMap&);
   RectProperty<int> _rect;

   //input
   virtual Handled _unhandled_input(InputEvent&){return false;}
   virtual void _register_parent_properties(){};

   void _is_extendable(){static_assert(true);}
   virtual std::string _get_static_constexpr_typename(){return TYPE_NAME;}

   //convenience
   void _publishSize(){WidgetResizeEvent event(toEventPublisher());publish<decltype(event)>(event);}

   bool _has_inited = false; //set true THE FIRST TIME a widget enters the tree. Can do constructors of children and other stuff requiring shared_from_this();
   bool isLayout = false;
   bool isInLayout = false;
   bool acceptsHover = false;
   ReyEngine::Size<int> maxSize = {ReyEngine::MaxInt, ReyEngine::MaxInt};
   ReyEngine::Size<int> minSize = {ReyEngine::MinInt, ReyEngine::MinInt};

   //editor stuff
public:
   void setInEditor(bool state){_isEditorWidget = state;}
   bool isInEditor(){return _isEditorWidget;}
   void setEditorSelected(bool selected){_editor_selected = selected;}
   bool isEditorSelected(){return _editor_selected;}
protected:
   bool _isEditorWidget = false; //true if this is a widget THE USER HAS PLACED IN THE EDITOR WORKSPACE (not a widget that the editor uses for normal stuff)
   bool _editor_selected = false; // true when the object is *selected* in the editor
   static constexpr int GRAB_HANDLE_SIZE = 10;
   ReyEngine::Rect<int> _getGrabHandle(int index);// 0-3 clockwise starting in top left (TL,TR,BR,BL)
   int _editor_grab_handles_dragging = -1; //which grab handle is being drug around
   ChildMap _children;
   ChildOrder _childrenOrdered;
   std::vector<std::shared_ptr<Component>> _components;

   AnchorProperty _anchor;
   void rename(WidgetPtr& child, const std::string& newName);
   const std::string _typeName; //can't just use static constexpr TYPE_NAME since we need to know what the type is if using type-erasure

   std::weak_ptr<BaseWidget> _parent;
   ///If this widget is the root of a scene, then the rest of the scene data is here.
   std::optional<std::shared_ptr<Scene>> _scene;
   bool _request_delete = false; //true when we want to remove this object from the tree
   bool _hovered = false; //true when hovered, set by application
   bool _visible = true; //whether to show the widget (and its children)
   std::recursive_mutex _childLock;
   bool _scheduled_for_deletion = false; // true when the widget has been scheduled for deletion but is not yet deleted.
   ReyEngine::Pos<double> _renderOffset; //used for different rendering modes. does not offset position.

   Handled _process_unhandled_input(InputEvent&); //pass input to children if they want it and then process it for ourselves if necessary
   Handled _process_unhandled_editor_input(InputEvent&); //pass input to children if they want it and then process it for ourselves if necessary ONLY FOR EDITOR RELATED THINGS (grab handles mostly)
   InputFilter inputFilter = InputFilter::INPUT_FILTER_PASS_AND_PROCESS;
   std::shared_ptr<Style::Theme> theme;

   bool _isRoot = false;
   friend class Window;
   friend class Application;
};