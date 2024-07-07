#include "BaseBody.h"
#include "Logger.h"
#include "Application.h"
#include "TypeContainer.h"

using namespace std;
using namespace ReyEngine;
using namespace Internal;

/////////////////////////////////////////////////////////////////////////////////////////
template <>
void TypeContainer<BaseBody>::_on_child_added_immediate(ChildPtr &) {

}

/////////////////////////////////////////////////////////////////////////////////////////
void Internal::Renderer3D::renderer3DChain() {
   renderer3DBegin();
   //front render
   for (const auto& child : _container.getChildren()){
      child->renderable3DChain();
   }
   renderer3DEnd();
   renderer3DEditorFeatures();
}

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
template <>
optional<shared_ptr<BaseBody>> Internal::TypeContainer<BaseBody>::addChild(ChildPtr &parent, ChildPtr &child) {
   if (child == parent){
      stringstream ss;
      ss << "Cannot add widget " << child->getName() << " to itself!" << endl;
      Logger::error() << ss.str();
      throw std::runtime_error(ss.str());
      return nullopt;
   }
   auto lock = std::scoped_lock<std::recursive_mutex>(_childLock);
   auto found = getChild(child->getName());
   if (found){
      stringstream ss;
      ss << "Container " << parent->getName() << " already has a child with name <" << child->getName() << ">";
      Logger::error() << ss.str();
      throw std::runtime_error(ss.str());
      return nullopt;
   }
   if (child->getComponentContainer().getParent().lock()){
      stringstream ss;
      ss << "Widget " << child->getName() << " already has a parent! It needs to be removed from its existing parent first!";
      Logger::error() << ss.str();
      throw std::runtime_error(ss.str());
      return nullopt;
   }
   //call immediate callback
   _on_child_added_immediate(child);
   auto& me = toContainedType();
   Logger::debug() << "Registering child " << child->getName() << " to parent " << parent->getName() << endl;
   Application::registerForEnterTree(child, me);

   return child;
}

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
void Internal::Renderable3D::renderable3DChain(){
   if (!_visible) return;
   render3DBegin();
   render3D();
   for (const auto& child : getChildren()){
      child->renderable3DChain();
   }
   render3DEnd();
   renderable3DEditorFeatures();
}
