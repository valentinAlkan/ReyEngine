#pragma once
#include "raylib.h"
#include "DrawInterface.h"
#include "Property.h"
#include "TypeManager.h"
#include "InputManager.h"
#include <utility>
#include <vector>
#include <memory>
#include <mutex>
#include <map>
#include <optional>
#include <unordered_set>
#include <unordered_map>

using Handled = bool;

#define GFCSDRAW_SERIALIZER(CLASSNAME, PARENT_CLASSNAME) \
   public:                                           \
   static std::shared_ptr<BaseWidget> CLASSNAME::deserialize(const std::string& instanceName, PropertyPrototypeMap& properties) { \
   auto retval = std::make_shared<CLASSNAME>(instanceName); \
   retval->BaseWidget::_deserialize(properties);        \
   return retval;}                                       \

#define GFCSDRAW_DEFAULT_CTOR(CLASSNAME, PARENT_CLASSNAME) \
   CLASSNAME(std::string name): CLASSNAME(std::move(name), #CLASSNAME){} \

#define GFCSDRAW_CUSTOM_CTOR(CLASSNAME, PARENT_CLASSNAME, ...) \
      CLASSNAME(std::string name, __VA_ARGS__): CLASSNAME(std::move(name), #CLASSNAME){} \

#define GFCSDRAW_OBJECT(CLASSNAME, PARENT_CLASSNAME) \
   GFCSDRAW_SERIALIZER(CLASSNAME, PARENT_CLASSNAME)  \
   GFCSDRAW_DEFAULT_CTOR(CLASSNAME, PARENT_CLASSNAME) \
   protected: \
   CLASSNAME(std::string name, std::string typeName): PARENT_CLASSNAME(name, std::move(typeName))

class Scene;
class  BaseWidget
: public std::enable_shared_from_this<BaseWidget>
, public PropertyContainer
{
   using WidgetPtr = std::shared_ptr<BaseWidget>;
   using ChildMap = std::map<std::string, WidgetPtr>;
   using fVec = GFCSDraw::Vec2<float>;
   using iVec = GFCSDraw::Vec2<int>;
   using dVec = GFCSDraw::Vec2<double>;
public:
   BaseWidget(std::string name, std::string typeName);
   ~BaseWidget();
   uint64_t getRid() const {return _rid;}
   std::string getName() const {return _name;}
   GFCSDraw::Rect<double> getRect() const {return _rect.value;}
   dVec getPos() const {return GFCSDraw::Vec2<double>(_rect.value.x, _rect.value.y);}
   dVec getGlobalPos() const;
   void setRect(const GFCSDraw::Rect<double>& r){_rect.value = r;}
   void setPos(double x, double y){_rect.value.x = x; _rect.value.y = y;}
   void setPos(const dVec& pos){_rect.value.x = pos.x; _rect.value.y = pos.y;}
   void setGlobalPos(const dVec&);
   bool setName(const std::string& name, bool append_index=false);
   std::string getTypeName(){return _typeName;}

   std::weak_ptr<BaseWidget> getParent(){return _parent;}
   const ChildMap& getChildren() const{return _children;}
   std::optional<WidgetPtr> getChild(const std::string& newName);

   void setProcess(bool process);
   WidgetPtr setFree(); //request to remove this widget from the tree at next available opportunity. Does not immediately delete it
                   // if you need your object to stop being processed immediately, use setFreeImmediately(), which could pause to syncrhonize threads.
   WidgetPtr setFreeImmediately(); // Pauses other threads and immediately removes objects from the tree.

   virtual void render() const = 0; //draw the widget
   bool isRoot();

   std::optional<WidgetPtr> addChild(WidgetPtr);
   std::optional<WidgetPtr> removeChild(WidgetPtr);

   bool operator==(const WidgetPtr&) const;

   static void registerType(std::string typeName, std::string parentType, bool isVirtual, Deserializer fx){TypeManager::registerType(typeName, parentType, isVirtual, fx);}
   std::string serialize();
protected:
   //override and setProcess(true) to allow processing
   virtual void _process(float dt){};
   // Drawing functions
   void renderChildren(); //draw the widget's children
   void _drawText(const std::string& text, const GFCSDraw::Vec2<int>& pos, int fontSize, Color color) const;
   void _drawTextCentered(const std::string& text, const GFCSDraw::Vec2<int>& pos, int fontSize, Color color) const;
   void _drawRectangle(const GFCSDraw::Rect<int>& rect, Color color) const;
   void _drawRectangleRounded(const GFCSDraw::Rect<int>& rect,  float roundness, int segments, Color color) const;
   void _drawRectangleRoundedLines(const GFCSDraw::Rect<float>& rect, float roundness, int segments, float lineThick, Color color) const;
   void registerProperties() override;
   void _deserialize(PropertyPrototypeMap&);
   RectProperty<double> _rect;

   //input
   virtual Handled _unhandled_input(InputEvent&){return false;}

private:
   uint64_t _rid; //unique identifier
   const std::string _typeName;
   std::string _name;
   BoolProperty _isProcessed;
   std::weak_ptr<BaseWidget> _parent; //todo: should be weak ptr
   ///If this widget is the root of a scene, then the rest of the scene data is here.
   std::optional<std::shared_ptr<Scene>> _scene;
   bool _request_delete = false; //true when we want to remove this object from the tree
   std::recursive_mutex _childLock;
   const std::lock_guard<std::recursive_mutex> childSafetyLock(){return std::lock_guard<std::recursive_mutex>(_childLock);}
   bool _scheduled_for_deletion = false; // true when the widget has been scheduled for deletion but is not yet deleted.

   Handled _process_unhandled_input(InputEvent&); //pass input to children if they want it and then process it for ourselves if necessary
   InputFilter inputFilter = InputFilter::INPUT_FILTER_PASS_AND_PROCESS;

   ChildMap _children;
   friend class Window;
};