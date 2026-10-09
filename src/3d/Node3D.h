#pragma once
#include "Drawable3D.h"
#include "Event.h"
#include "Easings.h"
#include "InputManager.h"

namespace ReyEngine {
   class Node3D;
   /// Where a ray from the camera struck a node (see Canvas3D::pick). All positions are in scene space.
   struct Hit3D {
      Node3D* node = nullptr;
      Ray3D ray;
      R_FLOAT distance = 0;
      [[nodiscard]] Pos3<R_FLOAT> getScenePos() const {return ray.at(distance);}
      [[nodiscard]] Pos3<R_FLOAT> getLocalPos() const; //where the hit occurred in the space of the node that was hit
   };

   /// The 3D counterpart of Widget: the base class for anything that lives in a Canvas3D's scene.
   /// On its own it draws nothing, so it also serves as a grouping node to move several children at once.
   ///
   /// Input works as it does for widgets: every node is offered every event, children before parents by default
   /// (see InputFilter), through _unhandled_input. For mouse events, MouseEvent::isInside() says whether the mouse is
   /// over this node or one of its descendants (the nearest pickable node under the mouse, so nodes hidden behind
   /// others are not "inside"), and getLocalRay()/getLocalHitPos() give the mouse ray and where it struck, in this
   /// node's local space.
   class Node3D
   : public Internal::Drawable3D
   , public Internal::Tree::Processable
   , public EventPublisher
   , public EventSubscriber
   , public Easable
   , public Internal::InputReceiver
   {
   public:
      EVENT_ARGS(TransformChangedEvent, 329876, const Transform3D& oldTransform)
      , oldTransform(oldTransform)
      {}
         Transform3D oldTransform;
      };
      EVENT_ARGS(EventNameChanged, 329877, const std::string& prevName)
      , prevName(prevName)
      {}
         std::string prevName;
      };
      REYENGINE_OBJECT(Node3D)
      [[nodiscard]] std::string getName() const override {return ReyObject::getName();}
      [[nodiscard]] std::optional<Canvas*> getInputCanvas() override;
      Node3D() = default;
      ~Node3D() override = default;
      [[nodiscard]] std::optional<Node3D*> getParentNode3D() const;
      [[nodiscard]] bool getIsRendering() const; //determines if this node and all its 3D ancestors are being drawn
      [[nodiscard]] bool isHovered() const;
      /// whether the mouse can hit this node. Unpickable nodes are transparent to the mouse, but their children are not.
      /// (Unpickable nodes are still offered input, as widgets outside the mouse are; it's just never "inside" them.)
      void setPickable(bool pickable){_pickable = pickable;}
      [[nodiscard]] bool getPickable() const {return _pickable;}
      /// bounds of whatever this node draws, in local space. Nodes with no bounds can't be hit by the mouse.
      [[nodiscard]] virtual std::optional<Box<R_FLOAT>> getLocalBounds() const {return {};}
      /// distance along a scene space ray to where it hits this node (not its children), if it does.
      /// The default tests getLocalBounds() (as an oriented box); override for something more precise.
      [[nodiscard]] virtual std::optional<R_FLOAT> intersectRay(const Ray3D& sceneRay) const;
   protected:
      void render3D(RenderContext3D&) const override {}
      virtual Handled _processInput(const InputEvent& e); //offer input to this node and its descendants
      virtual void _on_transform_changed(){}
      virtual void _on_visibility_changed(){}
      bool _pickable = true;
      void _on_renamed(const std::string& oldName) override {publish(EventNameChanged(this, oldName));}
   private:
      //offer input to a child node, localized to its space. The 3D counterpart of what Widget does with ScopeTransformer
      static Handled __pass_input(Node3D* child, const InputEvent& e);
      //whether the node the mouse is over (as picked by our Canvas3D) is this node or one of its descendants
      [[nodiscard]] bool __contains_mouse_pick() const;
      void __on_visibility_changed() override {_on_visibility_changed();}
      void __on_transform_changed(const Transform3D& oldTransform) override {
         _on_transform_changed();
         publish(TransformChangedEvent(this, oldTransform));
      }
      friend class Canvas3D;
   };
}
