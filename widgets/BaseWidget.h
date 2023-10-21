#pragma once
#include "raylib.h"
#include "DrawInterface.h"
#include "Property.h"
#include "TypeManager.h"
#include "InputManager.h"
#include "EventManager.h"
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

#define GFCSDRAW_DECLARE_STATIC_CONSTEXPR_TYPENAME(TYPENAME) \
static constexpr char TYPE_NAME[] = #TYPENAME;               \
std::string _get_static_constexpr_typename() override {return TYPE_NAME;}

#define GFCSDRAW_SERIALIZER(CLASSNAME, PARENT_CLASSNAME) \
   public:                                           \
   static std::shared_ptr<BaseWidget> deserialize(const std::string& instanceName, PropertyPrototypeMap& properties) { \
   CTOR_RECT = {0,0,0,0}; \
   auto retval = std::make_shared<CLASSNAME>(instanceName, r); \
   retval->BaseWidget::_deserialize(properties);        \
   return retval;}                                       \

#define GFCSDRAW_PROTECTED_CTOR(CLASSNAME, PARENT_CLASSNAME) \
   CLASSNAME(const std::string& name, const std::string& typeName, CTOR_RECT): PARENT_CLASSNAME(name, typeName, r)

#define GFCSDRAW_DEFAULT_CTOR(CLASSNAME) \
   CLASSNAME(const std::string& name, CTOR_RECT): CLASSNAME(name, _get_static_constexpr_typename(), r){}

#define GFCSDRAW_OBJECT(CLASSNAME, PARENT_CLASSNAME)      \
public:                                                   \
   GFCSDRAW_DECLARE_STATIC_CONSTEXPR_TYPENAME(CLASSNAME)  \
   GFCSDRAW_SERIALIZER(CLASSNAME, PARENT_CLASSNAME)       \
   GFCSDRAW_DEFAULT_CTOR(CLASSNAME)                       \
protected:                                                \
   void _register_parent_properties() override{           \
      PARENT_CLASSNAME::_register_parent_properties();    \
      PARENT_CLASSNAME::registerProperties();             \
   }                                                      \
   GFCSDRAW_PROTECTED_CTOR(CLASSNAME, PARENT_CLASSNAME)   \

class Scene;
class  BaseWidget
: public inheritable_enable_shared_from_this<BaseWidget>
, public EventSubscriber
, public EventPublisher
, public PropertyContainer
{
   using WidgetPtr = std::shared_ptr<BaseWidget>;
   using ChildMap = std::map<std::string, WidgetPtr>;
   using fVec = GFCSDraw::Vec2<float>;
   using iVec = GFCSDraw::Vec2<int>;
   using dVec = GFCSDraw::Vec2<double>;
public:
   static constexpr char TYPE_NAME[] = "BaseWidget";
   BaseWidget(const std::string& name, const std::string& typeName, GFCSDraw::Rect<float> rect);
   ~BaseWidget();
   uint64_t getRid() const {return _rid;}
   std::string getName() const {return _name;}
   GFCSDraw::Rect<double> getRect() const {return _rect.value;}
   dVec getPos() const {return {getRect().x, getRect().y};}
   dVec getGlobalPos() const;
   dVec globalToLocal(const dVec& global) const{return global - getGlobalPos();}
   dVec localToGlobal(const dVec& local) const {return local + getGlobalPos();}
   void setRect(const GFCSDraw::Rect<double>& r){_rect.value = r; _on_rect_changed();}
   void setPos(double x, double y){_rect.value.x = x; _rect.value.y = y; _on_rect_changed();}
   void setPos(const dVec& pos){_rect.value.x = pos.x; _rect.value.y = pos.y; _on_rect_changed();}
   void setGlobalPos(const dVec&);
   bool isInside(const dVec& point){return _rect.value.toSizeRect().isInside(point);}
   bool setName(const std::string& name, bool append_index=false);
   std::string getTypeName(){return _typeName;}

   std::weak_ptr<BaseWidget> getParent(){return _parent;}
   const ChildMap& getChildren() const{return _children;}
   std::optional<WidgetPtr> getChild(const std::string& newName);

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
   std::optional<WidgetPtr> removeChild(WidgetPtr);

   bool operator==(const WidgetPtr&) const;

   template <typename T> bool is_base_of(){return std::is_base_of_v<BaseWidget, T>;}
   static void registerType(const std::string& typeName, const std::string& parentType, bool isVirtual, Deserializer fx){TypeManager::registerType(typeName, parentType, isVirtual, fx);}
   std::string serialize();
protected:
   std::shared_ptr<BaseWidget> toBaseWidget(){return inheritable_enable_shared_from_this<BaseWidget>::shared_from_this();}
   virtual void _on_application_ready(){};
   virtual void _on_rect_changed(){}
   //override and setProcess(true) to allow processing
   virtual void _process(float dt){};
   // Drawing functions
   virtual void renderBegin(GFCSDraw::Vec2<float>& textureOffset){}
   void renderChildren(GFCSDraw::Vec2<float>& textureOffset) const; //draw the widget's children
   void renderChain(GFCSDraw::Vec2<float>& textureOffset);
   virtual void renderEnd(){}
   GFCSDraw::Vec2<float> getTextureRenderModeOffset(){return _textureRenderModeOffset;}
   void renderTextureOffsetApply(GFCSDraw::Vec2<float>& textureOffset){}
   void renderTextureOffsetReset(GFCSDraw::Vec2<float>& textureOffset){}
   void _drawText(const std::string& text, const GFCSDraw::Vec2<int>& pos, int fontSize, Color color) const;
   void _drawTextCentered(const std::string& text, const GFCSDraw::Vec2<int>& pos, int fontSize, Color color) const;
   void _drawRectangle(const GFCSDraw::Rect<int>& rect, Color color) const;
   void _drawRectangleRounded(const GFCSDraw::Rect<int>& rect,  float roundness, int segments, Color color) const;
   void _drawRectangleRoundedLines(const GFCSDraw::Rect<float>& rect, float roundness, int segments, float lineThick, Color color) const;
   void _drawRectangleGradientV(const GFCSDraw::Rect<int>& rect, Color color1, Color color2) const;
   void registerProperties() override;
   void _deserialize(PropertyPrototypeMap&);
   RectProperty<double> _rect;

   //input
   virtual Handled _unhandled_input(InputEvent&){return false;}
   virtual void _register_parent_properties(){};

   void _is_extendable(){static_assert(true);}
   virtual std::string _get_static_constexpr_typename(){return TYPE_NAME;}
private:
   uint64_t _rid; //unique identifier
   const std::string _typeName; //can't just use static constexpr TYPE_NAME since we need to know what the type is if using type-erasure
   std::string _name;
   BoolProperty _isProcessed;
   std::weak_ptr<BaseWidget> _parent; //todo: should be weak ptr
   ///If this widget is the root of a scene, then the rest of the scene data is here.
   std::optional<std::shared_ptr<Scene>> _scene;
   bool _request_delete = false; //true when we want to remove this object from the tree
   std::recursive_mutex _childLock;
   const std::lock_guard<std::recursive_mutex> childSafetyLock(){return std::lock_guard<std::recursive_mutex>(_childLock);}
   bool _scheduled_for_deletion = false; // true when the widget has been scheduled for deletion but is not yet deleted.

   GFCSDraw::Vec2<float> _textureRenderModeOffset; //used for texture rendering mode

   Handled _process_unhandled_input(InputEvent&); //pass input to children if they want it and then process it for ourselves if necessary
   InputFilter inputFilter = InputFilter::INPUT_FILTER_PASS_AND_PROCESS;

   ChildMap _children;
   friend class Window;
   friend class Application;
};