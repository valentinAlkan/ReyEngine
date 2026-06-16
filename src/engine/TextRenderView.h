#pragma once
#include "TrString.h"
#include "Widget.h"

namespace ReyEngine {
   template <typename T>
   class TextRenderModel {
   public:
      TextRenderModel(): _text(std::make_shared<T>()){}
      TextRenderModel(const T& text): _text(std::make_shared<T>(text)){}
      TextRenderModel(std::shared_ptr<T>& text): _text(text){}
      TextRenderModel(const TextRenderModel& other): _text(other._text) {}
      void setText(const T& text){*_text = text;}
      [[nodiscard]] const T& getText() const{return *_text;}
      void clear(){setText("");}
   private:
      std::shared_ptr<T> _text;
   };

   class TextRenderView : public Widget, public TextRenderModel<TrString> {
   public:
      REYENGINE_OBJECT(TextRenderView)
      TextRenderView(std::shared_ptr<TextRenderView>& other): TextRenderModel(*other){}
      TextRenderView(auto&&... args): TextRenderModel(std::forward<decltype(args)>(args)...){}
      ~TextRenderView() override = default;
   protected:
      void _init() override;
      void render2D(RenderContext&) const override;
   };
}
