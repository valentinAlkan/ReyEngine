#pragma once
#include "DrawInterface.h"
#include "Event.h"
#include "Component.h"

namespace ReyEngine {
   class BaseBody;
   namespace Internal {
      struct Transform3D{

      };

      struct Transform3DProperty : public Property<Transform3D>{
         using Property<Transform3D>::operator=;
         Transform3DProperty(const std::string& instanceName, Transform3D&& defaultvalue)
         : Property(instanceName, PropertyTypes::Int, std::move(defaultvalue))
         {}
         std::string toString() const override {throw std::runtime_error("Not implemented"); return"";}
         Transform3D fromString(const std::string& str) override { return Transform3D();}
      };


      class BaseBodyContainer : public virtual PropertyContainer {
         using ChildIndex = unsigned long;
         using BodyPtr = std::shared_ptr<BaseBody>;
         using ChildMap = std::map<std::string, std::pair<ChildIndex, BodyPtr>>;
         using ChildOrder = std::vector<BodyPtr>;
      protected:
         ChildMap _childMap3D;
         ChildOrder _childOrder3D;
      };

      //Something which renders 3D objects but is not a 3D body in itself
      class Renderer3D : public virtual BaseBodyContainer {
      protected:
         virtual void renderer3DBegin(){};
         virtual void renderer3DChain();
         virtual void renderer3DEnd(){};
         virtual void renderer3DEditorFeatures(){}
      };

      // Something which has volume is able to be rendered in 3D along with its children.
      class Renderable3D : public virtual Renderer3D {
      protected:
         virtual void render3DBegin(){};
         virtual void render3D(){};
         virtual void render3DEnd(){};
         virtual void renderable3DChain();
         virtual void renderable3DEditorFeatures(){}
         BoolProperty _visible;
         Transform3D _transform;
      };
   }


   //Combines the functionality above
   class BaseBody
   : public inheritable_enable_shared_from_this<BaseBody>
   , public virtual Internal::Renderable3D
   , public virtual Internal::Renderer3D
   , public Component
   , public EventPublisher
   , public EventSubscriber
   {
   protected:

      friend class Internal::Renderer3D;



   };
}
