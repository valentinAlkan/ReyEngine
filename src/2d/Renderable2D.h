#pragma once
#include "ReyEngine.h"
#include "Component.h"
#include "Positionable2D.h"
#include "Theme.h"

namespace ReyEngine{
   class Canvas;
}

namespace ReyEngine::Internal {
   //Something which renders 2D objects but is not a 3D body in itself
   class Renderable2D;
   class Renderer2D : public TypeContainer<Renderable2D>{
   public:
      using TypeContainer<Renderable2D>::getChildren;
//      std::optional<std::shared_ptr<Canvas>> getViewport();
   protected:
      Renderer2D(const std::string& name, const std::string& typeName, Canvas* canvas = nullptr)
      : TypeContainer<Renderable2D>(name, typeName)
      , NamedInstance(name, typeName)
      , canvas(canvas)
      {}
      virtual void renderer2DBegin(){};
      virtual void renderer2DChain();
      virtual void renderer2DEnd(){};
      virtual void renderer2DEditorFeatures(){}
      Canvas* canvas;
      const bool isCanvas = false;
   };

   // Something which has volume is able to be rendered in 3D along with its children.
   class Renderable2D : public Renderer2D, public Positionable2D<R_FLOAT>{
   public:
      using TypeContainer<Renderable2D>::getChildren;
      void setVisible(bool visible){_visible = visible;}
      bool getVisible() const {return _visible;}

      //drawing functions
      void drawLine(const Line<R_FLOAT>&, float lineThick, const ColorRGBA&) const;
      void drawArrow(const Line<R_FLOAT>&, float lineThick, const ColorRGBA&, float headSize=20) const;
      void drawText(const std::string& text, const Pos<R_FLOAT>& pos, const ReyEngineFont& font) const;
      void drawText(const std::string& text, const Pos<R_FLOAT>& pos, const ReyEngineFont& font, const ColorRGBA& color, R_FLOAT size, R_FLOAT spacing) const;
      void drawTextCentered(const std::string& text, const Pos<R_FLOAT>& pos) const;
      void drawTextCentered(const std::string& text, const Pos<R_FLOAT>& pos, const ReyEngineFont& font) const;
      void drawTextCentered(const std::string& text, const Pos<R_FLOAT>& pos, const ReyEngineFont& font, const ColorRGBA& color, R_FLOAT size, R_FLOAT spacing) const;
      void drawRectangle(const Rect<R_FLOAT>& rect, const ColorRGBA& color) const;
      void drawRectangleLines(const Rect<R_FLOAT>& rect, float lineThick, const ColorRGBA& color) const;
      void drawRectangleRounded(const Rect<R_FLOAT>& rect,  float roundness, int segments, const ColorRGBA& color) const;
      void drawRectangleRoundedLines(const Rect<R_FLOAT>& rect, float roundness, int segments, float lineThick, const ColorRGBA& color) const;
      void drawRectangleGradientV(const Rect<R_FLOAT>& rect, const ColorRGBA& color1, const ColorRGBA& color2) const;
      void drawCircle(const Circle&, const ColorRGBA&) const;
      void drawCircleLines(const Circle&, const ColorRGBA&) const;
      void drawCircleSectorLines(const CircleSector&, const ColorRGBA&, int segments) const;
      void drawRenderTarget(const RenderTarget&, const Pos<R_FLOAT>&, const ColorRGBA&) const;
      void drawRenderTargetRect(const RenderTarget&, const Rect<R_FLOAT>&, const Rect<R_FLOAT>&, const ColorRGBA&) const;
      void drawTextureRect(const ReyTexture&, const Rect<R_FLOAT>& src, const Rect<R_FLOAT>& dst, float rotation, const ColorRGBA& tint) const;
      void startScissor(const Rect<R_FLOAT>&) const;
      void stopScissor() const;
   protected:
      Renderable2D(const std::string& name, const std::string& typeName)
      : Renderer2D(name, typeName)
      , Positionable2D<R_FLOAT>(_size.value, _transform.value)
      , NamedInstance(name, typeName)
      , theme(new Style::Theme())
      , PROPERTY_DECLARE(_visible, true)
      , PROPERTY_DECLARE(_size)
      , PROPERTY_DECLARE(_transform)
      {}
      virtual void render2DBegin(){};
      virtual void render2D() const = 0;
      virtual void render2DEnd(){};
      virtual void render2DChain();
      virtual void render2DEditorFeatures(){}

      virtual void _on_rect_changed(){} //called when the rect is manipulated
      virtual void _on_child_rect_changed(std::shared_ptr<BaseWidget>&){} //called when an immediate child's rect is manipulated (not descendent)


      BoolProperty _visible; //whether to show the widget (and its children)
      SizeProperty<R_FLOAT> _size;
      Transform2DProperty _transform;
      std::shared_ptr<Style::Theme> theme;
      friend class Renderer2D;
      friend class TypeContainer<Renderable2D>;
   };
}