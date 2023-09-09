#pragma once
#include "raylib.h"
#include "DrawInterface.h"
#include <utility>
#include <vector>
#include <memory>
#include <optional>

class BaseWidget {
   using ChildVector = std::vector<std::shared_ptr<BaseWidget>>;
   using WidgetPtr = std::shared_ptr<BaseWidget>;
   using fVec = GFCSDraw::Vec2<float>;
   using iVec = GFCSDraw::Vec2<int>;
   using dVec = GFCSDraw::Vec2<double>;
public:
   BaseWidget(std::string name, WidgetPtr parent = nullptr)
   :_name(std::move(name))
   , _parent(parent ? parent : std::optional<WidgetPtr>(std::nullopt))
   {}

   std::string getName() const {return _name;}
   GFCSDraw::Rect<double> getRect() const {return _rect;}
   dVec getPos() const {return GFCSDraw::Vec2<double>(_rect.x, _rect.y);}
   dVec getGlobalPos() const;
   void setRect(const GFCSDraw::Rect<double>& r){_rect = r;}
   void setPos(const dVec& pos){_rect.x = pos.x; _rect.y = pos.y;}
   std::optional<bool> setName(const std::string& name, bool append_index=false);

   std::optional<WidgetPtr> getParent(){return _parent;}
   const ChildVector& getChildren() const{return _children;}
   std::optional<WidgetPtr> getChild(const std::string& newName);

   void setProcess();

   virtual void render() const = 0; //draw the widget
   bool isRoot();

protected:
   //override and setProcess(true) to allow processing
   virtual void _process(float dt){};
   // Drawing functions
   void renderChildren(); //draw the widget's children
   void _drawText(const std::string& text, const GFCSDraw::Vec2<int>& pos, int fontSize, Color color) const;
private:
   std::string _name;
   GFCSDraw::Rect<double> _rect;
   std::vector<std::shared_ptr<BaseWidget>> _children;
   std::optional<std::shared_ptr<BaseWidget>> _parent;
   bool _request_delete = false; //true when we want to remove this object from the tree
   friend class Window;
};