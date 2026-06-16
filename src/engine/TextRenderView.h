#pragma once
#include "TrString.h"
#include "Widget.h"

namespace ReyEngine {
   class TextRenderModel {
   public:
      TextRenderModel(const TrString& text)
      : _text(text)
      {}
      TextRenderModel(const TextRenderModel& other): _text(other._text) {}
      void setText(const TrString& text);
      const TrString& getText() const{return _text;}
   private:
      TrString _text;

   };


   class TextRenderView : public Widget {
   public:
      REYENGINE_OBJECT(TextRenderView)
      TextRenderView(const TrString& text = "") : _renderModel(text){}
      ~TextRenderView() override = default;
      void setText(const TrString& newText);
      [[nodiscard]] TrString getText() const {
         auto& text = _renderModel.getText();
         if (text.getLanguage() != _language) {
            text.translate(_language);
         }
         return text;
      }
      void clear(){setText("");}
   protected:
      void _init() override;
      void render2D(RenderContext&) const override;
      TextRenderModel _renderModel;
      Localization::Language* _language;
   };
}
