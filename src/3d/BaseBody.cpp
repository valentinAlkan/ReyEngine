#include "BaseBody.h"
#include "Logger.h"
#include "Application.h"

using namespace std;
using namespace ReyEngine;

//template specializations
template<> BaseBody& Internal::TypeContainer<BaseBody>::toContainedType(){
   return (BaseBody&)(*this);
};

/////////////////////////////////////////////////////////////////////////////////////////
template <>
optional<shared_ptr<BaseBody>> Internal::TypeContainer<BaseBody>::getChild(const std::string &name) {
   auto lock = std::scoped_lock<std::recursive_mutex>(_childLock);
   auto found = _childMap3D.find(name);
   if (found == _childMap3D.end()) {
      return nullopt;
   }

   return {found->second.second};
}


/////////////////////////////////////////////////////////////////////////////////////////
void Internal::Renderer3D::renderer3DChain() {
   renderer3DBegin();
   //front render
   for (const auto& child : _childOrder3D){
      child->renderable3DChain();
   }
   renderer3DEnd();
   renderer3DEditorFeatures();
}

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
template <>
optional<shared_ptr<BaseBody>> Internal::TypeContainer<BaseBody>::addChild(ChildPtr& child){
   if (*child == *this){
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
      ss << "Widget " << getName() << " already has a child with name <" << child->getName() << ">";
      Logger::error() << ss.str();
      throw std::runtime_error(ss.str());
      return nullopt;
   }
   if (child->getParent().lock()){
      stringstream ss;
      ss << "Widget " << child->getName() << " already has a parent! It needs to be removed from its existing parent first!";
      Logger::error() << ss.str();
      throw std::runtime_error(ss.str());
      return nullopt;
   }
   //call immediate callback
   _on_child_added_immediate(child);
   auto& me = toContainedType();
   Logger::debug() << "Registering child " << child->getName() << " to parent " << getName() << endl;
//   Application::registerForEnterTree(child, me);

   return child;
}

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
void Internal::Renderable3D::renderable3DChain(){
   if (!_visible) return;
   render3DBegin();
   render3D();
   for (const auto& child : _childOrder3D){
      child->renderable3DChain();
   }
   render3DEnd();
   renderable3DEditorFeatures();
}
