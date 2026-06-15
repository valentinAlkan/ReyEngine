#pragma once
#include "Widget.h"

namespace ReyEngine {
   class TextRenderView : public Widget {
   public:
      REYENGINE_OBJECT(TextRenderView)
      TextRenderView(const std::string& text = "")
      : _text(text)
      {}
      ~TextRenderView() override = default;

      void setText(const std::string& newText);
      [[nodiscard]] std::string getText() const {return _text;}
      void clear(){setText("");}

   protected:
      void _init() override;
      void render2D(RenderContext&) const override;

      std::string _text;
   };
}
