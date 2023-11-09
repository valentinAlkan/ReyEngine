#pragma once
#include "raylib.h"
#include "DrawInterface.h"
#include "Property.h"
#include "TypeManager.h"
#include "InputManager.h"
#include "EventManager.h"
#include "Theme.h"
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

#define CTOR_RECT const GFCSDraw::Rect<float>& r
/////////////////////////////////////////////////////////////////////////////////////////
#define GFCSDRAW_DECLARE_STATIC_CONSTEXPR_TYPENAME(TYPENAME) \
static constexpr char TYPE_NAME[] = #TYPENAME;               \
std::string _get_static_constexpr_typename() override {return TYPE_NAME;}
/////////////////////////////////////////////////////////////////////////////////////////
#define GFCSDRAW_SERIALIZER(CLASSNAME, PARENT_CLASSNAME) \
   public:                                           \
   static std::shared_ptr<BaseWidget> deserialize(const std::string& instanceName, PropertyPrototypeMap& properties) { \
   CTOR_RECT = {0,0,0,0}; \
   auto retval = std::make_shared<CLASSNAME>(instanceName, r); \
   retval->BaseWidget::_deserialize(properties);        \
   return retval;}                                       \
/////////////////////////////////////////////////////////////////////////////////////////
#define GFCSDRAW_PROTECTED_CTOR(CLASSNAME, PARENT_CLASSNAME) \
   CLASSNAME(const std::string& name, const std::string& typeName, CTOR_RECT): PARENT_CLASSNAME(name, typeName, r)
/////////////////////////////////////////////////////////////////////////////////////////
#define GFCSDRAW_DEFAULT_CTOR(CLASSNAME, ...) \
   CLASSNAME(const std::string& name, CTOR_RECT): CLASSNAME(name, _get_static_constexpr_typename(), r, __VA_ARGS__){}
/////////////////////////////////////////////////////////////////////////////////////////
#define GFCSDRAW_REGISTER_PARENT_PROPERTIES(PARENT_CLASSNAME) \
protected:                                                \
   void _register_parent_properties() override{           \
      PARENT_CLASSNAME::_register_parent_properties();    \
      PARENT_CLASSNAME::registerProperties();             \
   }

#define GFCSDRAW_OBJECT(CLASSNAME, PARENT_CLASSNAME)  \
public:                                                   \
   GFCSDRAW_DECLARE_STATIC_CONSTEXPR_TYPENAME(CLASSNAME)  \
   GFCSDRAW_SERIALIZER(CLASSNAME, PARENT_CLASSNAME)       \
   GFCSDRAW_DEFAULT_CTOR(CLASSNAME)                       \
   GFCSDRAW_REGISTER_PARENT_PROPERTIES(PARENT_CLASSNAME)  \
   GFCSDRAW_PROTECTED_CTOR(CLASSNAME, PARENT_CLASSNAME)


class Scene;
class  BaseWidget
: public inheritable_enable_shared_from_this<BaseWidget>
, public EventSubscriber
, public EventPublisher
, public Style::Themeable
{
   using ChildIndex = unsigned long;
   using WidgetPtr = std::shared_ptr<BaseWidget>;
   using ChildMap = std::map<std::string, std::pair<ChildIndex, WidgetPtr>>;
   using ChildOrder = std::vector<std::shared_ptr<BaseWidget>>;
   using fVec = GFCSDraw::Vec2<float>;
   using iVec = GFCSDraw::Vec2<int>;
   using dVec = GFCSDraw::Vec2<double>;
public:

   struct WidgetResizeEvent : public Event<WidgetResizeEvent> {
      EVENT_CTOR_SIMPLE(WidgetResizeEvent, Event<WidgetResizeEvent>){
         size = publisher->toBaseWidget()->getSize();
      }
      GFCSDraw::Size<float> size;
   };

   static constexpr char TYPE_NAME[] = "BaseWidget";
   BaseWidget(const std::string& name, std::string  typeName, GFCSDraw::Rect<float> rect);
   ~BaseWidget();
   uint64_t getRid() const {return _rid;}
   std::string getName() const {return _name;}

   //rect stuff
   GFCSDraw::Rect<int> getRect() const {return _rect.value;}
   GFCSDraw::Rect<int> getGlobalRect() const {return {getGlobalPos().x, getGlobalPos().y, getSize().x, getSize().y};}
   GFCSDraw::Pos<int> getGlobalPos() const;
   GFCSDraw::Size<int> getChildRectSize() const; //get the smallest rectangle that contains all children, starting from 0,0. Does not include grandchildren.
   GFCSDraw::Pos<int> getPos() const {return {getRect().x, getRect().y};}
   GFCSDraw::Size<int> getSize() const {return getRect().size();}
   int getWidth(){return _rect.value.width;}
   int getHeight(){return _rect.value.height;}
   iVec getHeightRange() const {return {0, getRect().size().y};}
   iVec getWidthtRange() const {return {0, getRect().size().x};}
   void setRect(const GFCSDraw::Rect<int>& r){_rect.value = r;_on_rect_changed();WidgetResizeEvent event(toEventPublisher());publish<decltype(event)>(event);}
   void setPos(int x, int y){_rect.value.x = x; _rect.value.y = y; _on_rect_changed();WidgetResizeEvent event(toEventPublisher());publish<decltype(event)>(event);}
   void setPos(const GFCSDraw::Pos<int>& pos){_rect.value.x = pos.x; _rect.value.y = pos.y; _on_rect_changed();WidgetResizeEvent event(toEventPublisher());publish<decltype(event)>(event);}
   void setMaxSize(const GFCSDraw::Size<int>& size){maxSize = size;}
   void setMinSize(const GFCSDraw::Size<int>& size){minSize = size;}
   GFCSDraw::Size<int> getMinSize(){return minSize;}
   GFCSDraw::Size<int> getMaxSize(){return maxSize;}
   void setSize(const GFCSDraw::Size<int>& size){_rect.value.width = size.x; _rect.value.height = size.y; _on_rect_changed();WidgetResizeEvent event(toEventPublisher());publish<decltype(event)>(event);}
   void setWidth(int width){_rect.value.width = width; _on_rect_changed();WidgetResizeEvent event(toEventPublisher());publish<decltype(event)>(event);}
   void setHeight(int height){_rect.value.height = height; _on_rect_changed();WidgetResizeEvent event(toEventPublisher());publish<decltype(event)>(event);}

   iVec getLocalMousePos(){return globalToLocal(InputManager::getMousePos());}
   iVec globalToLocal(const dVec& global) const{return global - getGlobalPos();}
   iVec localToGlobal(const dVec& local) const {return local + getGlobalPos();}
   void setGlobalPos(const dVec&);
   bool isInside(const dVec& point){return _rect.value.toSizeRect().isInside(point);}
   bool setName(const std::string& name, bool append_index=false);
   bool setIndex(unsigned int newIndex);
   std::string getTypeName(){return _typeName;}

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
   bool isRoot();

   std::optional<WidgetPtr> addChild(WidgetPtr);
   std::optional<WidgetPtr> removeChild(const std::string& name, bool quiet = false); //quiet silences the output if child is not found.

   bool operator==(const WidgetPtr& other) const {if (other){return other->getRid()==_rid;}return false;}
   bool operator==(const BaseWidget& other) const{return other._rid == _rid;}

   template <typename T> bool is_base_of(){return std::is_base_of_v<BaseWidget, T>;}
   static void registerType(const std::string& typeName, const std::string& parentType, bool isVirtual, Deserializer fx){TypeManager::registerType(typeName, parentType, isVirtual, fx);}
   std::string serialize();
protected:
   std::shared_ptr<BaseWidget> toBaseWidget(){return inheritable_enable_shared_from_this<BaseWidget>::shared_from_this();}
   virtual void _on_application_ready(){}; //called when the main loop is starting, or immediately if that's already happened
   virtual void _init(){}; //run ONCE PER OBJECt when it enters tree for first time.
   virtual void _on_rect_changed(){} //called when the rect is manipulated
   virtual void _on_mouse_enter(){};
   virtual void _on_mouse_exit(){};
   virtual void _on_child_added_immediate(WidgetPtr&){} //Called immediately upon a call to addChild - DANGER: widget is not actually a child yet! It is (probably) a very bad idea to do much at all here. Make sure you know what you're doing.
   virtual void _on_child_added(WidgetPtr&){} // called at the beginning of the next frame after a child is added. Child is now owned by us. Safe to manipulate child.
   virtual void _on_enter_tree(){} //called every time a widget enters the tree
   virtual void _on_exit_tree(){}
   virtual void _on_child_removed(){}

   //override and setProcess(true) to allow processing
   virtual void _process(float dt){};

   // Drawing functions
   virtual void renderBegin(GFCSDraw::Pos<double>& textureOffset){}
   void renderChildren(GFCSDraw::Pos<double>& textureOffset) const; //draw the widget's children
   void renderChain(GFCSDraw::Pos<double>& textureOffset);
   virtual void renderEnd(){}
   GFCSDraw::Vec2<float> getRenderOffset() const {return _renderOffset;}
   void setRenderOffset(GFCSDraw::Pos<double>& offset){_renderOffset = offset;}
//   void renderTextureOffsetApply(GFCSDraw::Pos<float>& textureOffset){}
//   void renderTextureOffsetReset(GFCSDraw::Pos<float>& textureOffset){}
   void _drawText(const std::string& text, const GFCSDraw::Vec2<int>& pos, int fontSize, GFCSDraw::ColorRGBA color) const;
   void _drawTextCentered(const std::string& text, const GFCSDraw::Vec2<int>& pos, int fontSize, GFCSDraw::ColorRGBA color) const;
   void _drawRectangle(const GFCSDraw::Rect<int>& rect, GFCSDraw::ColorRGBA color) const;
   void _drawRectangleLines(const GFCSDraw::Rect<int>& rect, float lineThick, GFCSDraw::ColorRGBA color) const;
   void _drawRectangleRounded(const GFCSDraw::Rect<int>& rect,  float roundness, int segments, GFCSDraw::ColorRGBA color) const;
   void _drawRectangleRoundedLines(const GFCSDraw::Rect<int>& rect, float roundness, int segments, float lineThick, GFCSDraw::ColorRGBA color) const;
   void _drawRectangleGradientV(const GFCSDraw::Rect<int>& rect, GFCSDraw::ColorRGBA color1, GFCSDraw::ColorRGBA color2) const;
   void registerProperties() override;
   void _deserialize(PropertyPrototypeMap&);
   RectProperty<int> _rect;

   //input
   virtual Handled _unhandled_input(InputEvent&){return false;}
   virtual void _register_parent_properties(){};

   void _is_extendable(){static_assert(true);}
   virtual std::string _get_static_constexpr_typename(){return TYPE_NAME;}

   bool _has_inited = false; //set true THE FIRST TIME a widget enters the tree. Can do constructors of children and other stuff requiring shared_from_this();
   bool isLayout = false;
   bool isInLayout = false;
   bool acceptsHover = false;
   GFCSDraw::Size<int> maxSize = {INT_MAX, INT_MAX};
   GFCSDraw::Size<int> minSize = {0,0};
private:
   void rename(WidgetPtr& child, const std::string& newName);
   uint64_t _rid; //unique identifier
   const std::string _typeName; //can't just use static constexpr TYPE_NAME since we need to know what the type is if using type-erasure
   std::string _name;
   BoolProperty _isProcessed;
   std::weak_ptr<BaseWidget> _parent;
   ///If this widget is the root of a scene, then the rest of the scene data is here.
   std::optional<std::shared_ptr<Scene>> _scene;
   bool _request_delete = false; //true when we want to remove this object from the tree
   bool _hovered = false; //true when hovered, set by application
   std::recursive_mutex _childLock;
   bool _scheduled_for_deletion = false; // true when the widget has been scheduled for deletion but is not yet deleted.

   GFCSDraw::Pos<double> _renderOffset; //used for different rendering modes. does not offset position.

   Handled _process_unhandled_input(InputEvent&); //pass input to children if they want it and then process it for ourselves if necessary
   InputFilter inputFilter = InputFilter::INPUT_FILTER_PASS_AND_PROCESS;

   ChildMap _children;
   ChildOrder _childrenOrdered;
   friend class Window;
   friend class Application;
};