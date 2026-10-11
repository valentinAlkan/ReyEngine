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
std::optional<Canvas*> Node3D::getInputCanvas() {
   if (auto canvas = getCanvas3D()) return canvas.value();
   return {};
}

/////////////////////////////////////////////////////////////////////////////////////////
bool Node3D::isHovered() const {
   if (auto canvas = getCanvas3D()) return canvas.value()->getHover() == this;
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
bool Node3D::__contains_mouse_pick() const {
   auto canvas = getCanvas3D();
   if (!canvas) return false;
   for (const Node3D* node = canvas.value()->_mousePick; node; node = node->getParentNode3D().value_or(nullptr)) {
      if (node == this) return true;
   }
   return false;
}

/////////////////////////////////////////////////////////////////////////////////////////
Handled Node3D::__pass_input(Node3D* child, const InputEvent& e) {
   //transform mouse input into the child's coordinate space
   std::optional<MouseEvent::ScopeTransformer3D> xformer;
   if (auto mouse = e.isMouse()) xformer.emplace(*mouse.value(), child->getLocalTransform(), child->__contains_mouse_pick());
   return child->_processInput(e);
}

/////////////////////////////////////////////////////////////////////////////////////////
Handled Node3D::__pass_panel_input(Canvas* panel, const InputEvent& e) {
   std::optional<MouseEvent::ScopeTransformerToPanel> xformer;
   if (auto isMouse = e.isMouse()) {
      auto mouse = isMouse.value();
      //where the mouse ray crosses the panel. If it doesn't, the mouse is nowhere on it
      Pos<float> panelPos(-1, -1);
      bool isInside = false;
      if (auto ray = mouse->getLocalRay()) {
         if (auto distance = Canvas3D::intersectPanelPlane(ray.value())) {
            auto nodePos = ray->at(distance.value());
            panelPos = Canvas3D::nodeToPanelPos(panel, nodePos);
            //the mouse is only over the panel if it's the nearest thing hit. (Canvas3D picks panels as part of their node.)
            static constexpr R_FLOAT TOLERANCE = 1e-4;
            auto hit = mouse->getLocalHitPos();
            bool isNearest = hit && Vec3<R_FLOAT>(hit.value() - ray->origin).dot(ray->direction) >= distance.value() * (1 - TOLERANCE);
            isInside = isNearest && panel->getSizeRect().contains(panelPos);
         }
      }
      //a canvas ignores input outside of itself, so it wouldn't otherwise learn that the mouse has left whatever it was over
      if (!isInside && e.eventId == InputEventMouseHover::ID) panel->setHover(nullptr);
      xformer.emplace(*mouse, panelPos, isInside);
   }
   return static_cast<Widget*>(panel)->_processInput(e);
}

/////////////////////////////////////////////////////////////////////////////////////////
Handled Node3D::_processInput(const InputEvent& e) {
   if (!_visible) return nullptr;
   auto pass = [&]() -> Handled {
      for (auto& child : getChildren()) {
         if (auto isNode = child->as<Node3D>()) {
            if (auto handled = __pass_input(isNode.value(), e)) return handled;
         } else if (auto isPanel = child->as<Canvas>()) {
            if (auto handled = __pass_panel_input(isPanel.value(), e)) return handled;
         }
      }
      return nullptr;
   };
   auto publish = [&]() -> Handled {
      UnhandledInputEvent event(this, e);
      publishMutable(event);
      return event.handled;
   };
   return __filter_input(e, pass, publish);
}

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
Pos3<R_FLOAT> Hit3D::getLocalPos() const {
   return node ? node->toLocalPos(getScenePos()) : getScenePos();
}
