#pragma once
#include "ReyEngine.h"
#include "Component.h"
#include "Positionable2D.h"
#include "Theme.h"

namespace ReyEngine{
   class Canvas;
}

namespace ReyEngine::Internal {
   //Something which renders 3D objects but is not a 3D body in itself
   class Renderable2D;
   class Renderer2D : public TypeContainer<Renderable2D>{
   public:
      using TypeContainer<Renderable2D>::getChildren;
      std::optional<std::shared_ptr<Canvas>> getViewport();
   protected:
      Renderer2D(const std::string& name, const std::string& typeName)
      : TypeContainer<Renderable2D>(name, typeName)
      , NamedInstance(name, typeName)
      {}
      virtual void renderer2DBegin(){};
      virtual void renderer2DChain();
      virtual void renderer2DEnd(){};
      virtual void renderer2DEditorFeatures(){}
   };

   // Something which has volume is able to be rendered in 3D along with its children.
   class Renderable2D : public virtual Internal::Component, public Renderer2D, public Positionable2D<R_FLOAT>{
   public:
      using TypeContainer<Renderable2D>::getChildren;
      void setVisible(bool visible){_visible = visible;}
      bool getVisible() const {return _visible;}
   protected:
      Renderable2D(const std::string& name, const std::string& typeName)
      : Renderer2D(name, typeName)
      , Positionable2D<R_FLOAT>(_size.value, _transform.value)
      , Component(name, typeName)
      , NamedInstance(name, typeName)
      , theme(new Style::Theme())
      , PROPERTY_DECLARE(_visible, true)
      , PROPERTY_DECLARE(_size)
      , PROPERTY_DECLARE(_transform)
      {}
      virtual void render2DBegin(){};
      virtual void render2D() const;
      virtual void render2DEnd(){};
      virtual void renderable2DChain();
      virtual void renderable2DEditorFeatures(){}
      BoolProperty _visible; //whether to show the widget (and its children)
      SizeProperty<R_FLOAT> _size;
      Transform2DProperty _transform;
      std::shared_ptr<Style::Theme> theme;
      friend class Renderer2D;
      friend class TypeContainer<Renderable2D>;
   };
}