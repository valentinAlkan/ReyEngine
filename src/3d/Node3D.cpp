#include "Node3D.h"

using namespace std;
using namespace ReyEngine;

/////////////////////////////////////////////////////////////////////////////////////////
std::optional<Node3D*> Node3D::getParentNode3D() const {
   if (!_node) return {};
   if (auto parent = _node->getParent()) {
      if (auto isNode3D = parent->as<Node3D>()) return isNode3D.value();
   }
   return {};
}

/////////////////////////////////////////////////////////////////////////////////////////
bool Node3D::getIsRendering() const {
   if (!_visible) return false;
   if (auto parent = getParentNode3D()) return parent.value()->getIsRendering();
   return true;
}
