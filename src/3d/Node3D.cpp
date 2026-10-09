#include "Node3D.h"
#include "Canvas3D.h"

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

/////////////////////////////////////////////////////////////////////////////////////////
bool Node3D::isHovered() const {
   if (auto canvas = getCanvas3D()) return canvas.value()->getHover3D() == this;
   return false;
}

/////////////////////////////////////////////////////////////////////////////////////////
std::optional<R_FLOAT> Node3D::intersectRay(const Ray3D& sceneRay) const {
   auto bounds = getLocalBounds();
   if (!bounds) return {};
   //test in local space so the box rotates with the node
   auto global = getGlobalTransform();
   auto localRay = sceneRay.transform(global.inverse().matrix);
   if (auto localDistance = localRay.intersects(bounds.value())) {
      auto sceneHit = localRay.at(localDistance.value()).transform(global.matrix);
      return sceneHit.distanceTo(sceneRay.origin);
   }
   return {};
}

/////////////////////////////////////////////////////////////////////////////////////////
bool Node3D::__process_unhandled_input(const InputEvent& event, const Hit3D& hit) {
   if (_unhandled_input(event, hit)) return true;
   EventInput3D inputEvent(this, event, hit);
   publishMutable(inputEvent);
   return inputEvent.handled;
}

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
Pos3<R_FLOAT> Hit3D::getLocalPos() const {
   return node ? node->toLocalPos(getScenePos()) : getScenePos();
}
