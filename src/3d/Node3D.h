#pragma once
#include "Drawable3D.h"
#include "Event.h"
#include "Easings.h"

namespace ReyEngine {
   /// The 3D counterpart of Widget: the base class for anything that lives in a Canvas3D's scene.
   /// On its own it draws nothing, so it also serves as a grouping node to move several children at once.
   class Node3D
   : public Internal::Drawable3D
   , public Internal::Tree::Processable
   , public EventPublisher
   , public EventSubscriber
   , public Easable
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
      Node3D() = default;
      ~Node3D() override = default;
      [[nodiscard]] std::optional<Node3D*> getParentNode3D() const;
      [[nodiscard]] bool getIsRendering() const; //determines if this node and all its 3D ancestors are being drawn
   protected:
      void render3D(RenderContext3D&) const override {}
      virtual void _on_transform_changed(){}
      virtual void _on_visibility_changed(){}
      void _on_renamed(const std::string& oldName) override {publish(EventNameChanged(this, oldName));}
   private:
      void __on_visibility_changed() override {_on_visibility_changed();}
      void __on_transform_changed(const Transform3D& oldTransform) override {
         _on_transform_changed();
         publish(TransformChangedEvent(this, oldTransform));
      }
   };
}
