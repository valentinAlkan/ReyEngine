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
      template <typename T>
      class TypeContainer : public Component
      {
         using ChildIndex = unsigned long;
         using ChildPtr = std::shared_ptr<T>;
         using ChildMap = std::map<std::string, std::pair<ChildIndex, std::shared_ptr<T>>>;
         using ChildOrder = std::vector<ChildPtr>;
      public:
         TypeContainer(const std::string& instanceName)
         : Component(instanceName){}
         T& toContainedType();
         std::optional<ChildPtr> addChild(ChildPtr&);
         void removeChild3D(ChildPtr&);
         std::optional<ChildPtr> getChild(const std::string& name);
         std::weak_ptr<T> getParent(){return _parent;}
      protected:
         virtual void _on_child_added_immediate(ChildPtr&) = 0;


         ChildMap _childMap3D;
         ChildOrder _childOrder3D;
      private:
         std::weak_ptr<T> _parent;
         std::recursive_mutex _childLock;
      };

      //Something which renders 3D objects but is not a 3D body in itself
      class Renderer3D : public TypeContainer<BaseBody> {
      public:
         Renderer3D(const std::string& instanceName)
         : TypeContainer<BaseBody>(instanceName){}
      protected:
         virtual void renderer3DBegin(){};
         virtual void renderer3DChain();
         virtual void renderer3DEnd(){};
         virtual void renderer3DEditorFeatures(){}
         void _on_child_added_immediate(std::shared_ptr<BaseBody>&) override {};
      };

      // Something which has volume is able to be rendered in 3D along with its children.
      class Renderable3D : public Renderer3D {
      public:
         Renderable3D(const std::string& instanceName)
         : Renderer3D(instanceName)
         , _visible("visible")
         {}
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
   , public Internal::Renderable3D
   , public EventPublisher
   , public EventSubscriber
   {
   protected:
      friend class Internal::Renderer3D;



   };
}
