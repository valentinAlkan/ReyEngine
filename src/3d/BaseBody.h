#pragma once
#include "DrawInterface.h"
#include "Event.h"
#include "Component.h"
#include <mutex>

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


//      class BaseBody;


      //Something which renders 3D objects but is not a 3D body in itself
      class Renderable3D;
      class Renderer3D : public TypeContainerInterface<Renderable3D>{
      public:
         Renderer3D() : TypeContainerInterface<Renderable3D>(_container) {}
      protected:
         virtual void renderer3DBegin(){};
         virtual void renderer3DChain();
         virtual void renderer3DEnd(){};
         virtual void renderer3DEditorFeatures(){}
         TypeContainer<Renderable3D> _container;
      };

      // Something which has volume is able to be rendered in 3D along with its children.
      class Renderable3D : public Renderer3D {
         Renderable3D() : _visible("visible")
         {}
      protected:
         virtual void render3DBegin(){};
         virtual void render3D(){};
         virtual void render3DEnd(){};
         virtual void renderable3DChain();
         virtual void renderable3DEditorFeatures(){}
         BoolProperty _visible;
         Transform3D _transform;
         friend class Renderer3D;
      };
   }


   //Combines the functionality above
   class BaseBody : public Internal::Component {
      REYENGINE_OBJECT_BUILD_ONLY(BaseBody, Internal::Component){}
   protected:
      std::shared_ptr<BaseBody> toBaseBody();
      void _on_child_added_immediate();
      friend class Internal::Renderer3D;
   };
}
