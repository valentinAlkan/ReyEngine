#pragma once
#include "raylib.h"
#include "DrawInterface.h"
#include "Property.h"
#include <utility>
#include <vector>
#include <memory>
#include <mutex>
#include <map>
#include <optional>
#include <unordered_set>
#include <unordered_map>



class BaseWidget : public std::enable_shared_from_this<BaseWidget> {
   using WidgetPtr = std::shared_ptr<BaseWidget>;
   using ChildMap = std::map<std::string, WidgetPtr>;
   using fVec = GFCSDraw::Vec2<float>;
   using iVec = GFCSDraw::Vec2<int>;
   using dVec = GFCSDraw::Vec2<double>;
public:
   BaseWidget(std::string name, std::string typeName);
   uint64_t getRid() const {return _rid;}
   std::string getName() const {return _name;}
   GFCSDraw::Rect<double> getRect() const {return _rect;}
   dVec getPos() const {return GFCSDraw::Vec2<double>(_rect.x, _rect.y);}
   dVec getGlobalPos() const;
   void setRect(const GFCSDraw::Rect<double>& r){_rect = r;}
   void setPos(double x, double y){_rect.x = x; _rect.y = y;}
   void setPos(const dVec& pos){_rect.x = pos.x; _rect.y = pos.y;}
   bool setName(const std::string& name, bool append_index=false);
   std::string getTypeName(){return _typeName;}

   std::optional<WidgetPtr> getParent(){return _parent;}
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
protected:
   //override and setProcess(true) to allow processing
   virtual void _process(float dt){};
   // Drawing functions
   void renderChildren(); //draw the widget's children
   void _drawText(const std::string& text, const GFCSDraw::Vec2<int>& pos, int fontSize, Color color) const;

   virtual void registerProperties() = 0;
   void registerProperty(BaseProperty& property);
private:
   uint64_t _rid; //unique identifier
   const std::string _typeName;
   std::string _name;
   GFCSDraw::Rect<double> _rect;
   std::optional<WidgetPtr> _parent; //todo: should be weak ptr
   ///If this widget is the root of a scene, then the rest of the scene data is here.
   std::optional<std::unique_ptr<Scene>> _scene;
   bool _request_delete = false; //true when we want to remove this object from the tree
   std::recursive_mutex _childLock;
   const std::lock_guard<std::recursive_mutex> childSafetyLock(){return std::lock_guard<std::recursive_mutex>(_childLock);}
   bool _scheduled_for_deletion = false; // true when the widget has been scheduled for deletion but is not yet deleted.

   std::string serialize();
   void deserialize(std::map<std::string, std::string>);

   ChildMap _children;
   PropertyMap _properties;
   friend class Window;
};