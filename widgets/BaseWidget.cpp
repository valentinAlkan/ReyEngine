#include <algorithm>
#include "BaseWidget.h"
#include "Application.h"

using namespace std;
using namespace GFCSDraw;

/////////////////////////////////////////////////////////////////////////////////////////
std::optional<std::shared_ptr<BaseWidget>> BaseWidget::getChild(const std::string &childName) {
   for (const auto& child : _children){
      if (child->getName() == childName){
         return child;
      }
   }
   return nullopt;
}

/////////////////////////////////////////////////////////////////////////////////////////
std::optional<bool> BaseWidget::setName(const std::string& newName, bool append_index) {
   //if the child has a sibling by the same name, it cannot be renamed
   if (_parent && _parent.value()->getChild(newName)){
      //if we are allowed to, just append an index to the name (start at 2)
      if (append_index) {
         int index = 2;
         while (_parent.value()->getChild(newName + to_string(index))){
            index++;
         }
         _name = newName + to_string(index);
         return true;
      } else {
         return false;
      }
   }
   _name = newName;
   return true;
}

/////////////////////////////////////////////////////////////////////////////////////////
Vec2<double> BaseWidget::getGlobalPos() const {
   //sum up all our ancestors' positions and add our own to it
   auto offset = getPos();
   auto parent = _parent;
   while (parent && parent.value()) {
      offset.x += parent.value()->getPos().x;
      offset.y += parent.value()->getPos().y;
   }
   return offset;
}

/////////////////////////////////////////////////////////////////////////////////////////
void BaseWidget::renderChildren() {
   for (const auto& child : _children){
      child->renderChildren();
      child->render();
   }
}


/////////////////////////////////////////////////////////////////////////////////////////
void BaseWidget::setProcess() {
   if (isRoot()){

   }
}

/////////////////////////////////////////////////////////////////////////////////////////
bool BaseWidget::isRoot() {
   return Application::instance().getWindow()->getRootWidget().get() == this;
}

/////////////////////////////////////////////////////////////////////////////////////////
void BaseWidget::_drawText(const std::string &text, const GFCSDraw::Vec2<int> &pos, int fontSize, Color color) const{
   auto globalPos = getGlobalPos();
   DrawText(text.c_str(), (int)(pos.x + globalPos.x), (int)(pos.y + globalPos.y), fontSize, color);
}