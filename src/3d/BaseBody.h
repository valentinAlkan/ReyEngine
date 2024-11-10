#pragma once
#include "DrawInterface3D.h"
#include "Event.h"
#include "Component.h"
#include <mutex>

namespace ReyEngine {
   class BaseBody;
   class Viewport;
   namespace Internal {
      struct Transform3D{
          Vec3<float> position;

      };

      struct Transform3DProperty : public Property<Transform3D>{
         using Property<Transform3D>::operator=;
         Transform3DProperty(const std::string& instanceName, Transform3D&& defaultvalue)
         : Property(instanceName, PropertyTypes::Int, std::move(defaultvalue))
         {}
         std::string toString() const override {throw std::runtime_error("Not implemented"); return"";}
         Transform3D fromString(const std::string& str) override { return Transform3D();}
      };

      //Something which renders 3D objects but is not a 3D body in itself
      class Renderable3D;
      class Renderer3D : public TypeContainer<Renderable3D>{
      public:
         using TypeContainer<Renderable3D>::getChildren;
         std::optional<std::shared_ptr<Viewport>> getViewport();
         virtual std::optional<Collisions::RayHit3D> testRayClosest(const Collisions::Ray3D&);
         virtual std::vector<Collisions::RayHit3D> testRayAll(const Collisions::Ray3D){return {};};
      protected:
         Renderer3D(const std::string& name, const std::string& typeName)
         : TypeContainer<Renderable3D>(name, typeName)
         , NamedInstance(name, typeName)
         {}
         virtual void renderer3DBegin(){};
         virtual void renderer3DChain();
         virtual void renderer3DEnd(){};
         virtual void renderer3DEditorFeatures(){}
      };

      // Something which has volume is able to be rendered in 3D along with its children.
      class Renderable3D : public virtual Internal::Component, public Renderer3D {
      public:
         using TypeContainer<Renderable3D>::getChildren;
         void setVisible(bool visible){_visible = visible;}
      protected:
         Renderable3D(const std::string& name, const std::string& typeName)
         : Renderer3D(name, typeName)
         , Component(name, typeName)
         , NamedInstance(name, typeName)
         , _visible("visible")
         , _scale("scale", 1)
         , _tint("tint", Colors::none)
         {}
         virtual void render3DBegin(){};
         virtual void render3D() const;
         virtual void render3DEnd(){};
         virtual void renderable3DChain();
         virtual void renderable3DEditorFeatures(){}
         virtual void _init(){}
         std::optional<Collisions::RayHit3D> testRayClosest(const Collisions::Ray3D& ray) override;
         std::vector<Collisions::RayHit3D> testRayAll(const Collisions::Ray3D) override{return {};};
         Transform3D _transform;
         BoolProperty _visible;
         FloatProperty _scale;
         std::shared_ptr<Model3D> _model;
         std::shared_ptr<ReyTexture> _texture;
         ColorProperty _tint;
         friend class Renderer3D;
         friend class TypeContainer<Renderable3D>;
      };
   }


   //Combines the functionality above
class BaseBody : public Internal::Renderable3D {
public:
   friend class ReyEngine::Internal::Component;
   static constexpr char TYPE_NAME[] = "BaseBody";
   std::string _get_static_constexpr_typename() override {return TYPE_NAME;}
   using Internal::Renderable3D::getChildren;
protected:
   BaseBody(const std::string &name) : BaseBody(name, _get_static_constexpr_typename()) {}
protected:
   void _register_parent_properties()
   override{
      Internal::Component::_register_parent_properties();
      Internal::Component::registerProperties();
   }
   BaseBody(const std::string &name, const std::string &typeName)
   : Internal::Renderable3D(name, typeName)
   , NamedInstance(name, typeName)
   , Component(name, typeName)
   {}
   protected:
      void _on_application_ready() override{}
      std::shared_ptr<BaseBody> toBaseBody();
      friend class Internal::Renderer3D;
   };
}
