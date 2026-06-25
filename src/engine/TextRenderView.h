#pragma once
#include "TrString.h"
#include "Widget.h"

namespace ReyEngine {
   // A shared, observable text store. The model owns the text (via a shared_ptr so
   // several models/views can alias the same buffer) and is an EventPublisher: every
   // mutation fires EventTextChanged. Any number of TextRenderViews can hold the same
   // model shared_ptr and subscribe to it, so a change made through one view notifies
   // them all. The model is a plain object (not a Widget / tree node) - it can live as
   // a member and publish without being in the scene tree.
   template <typename T>
   class TextRenderModel : public EventPublisher {
   public:
      EVENT(EventTextChanged, 327634316934981){}};
      explicit TextRenderModel(std::shared_ptr<T> text): _text(std::move(text)){}
      ~TextRenderModel() override = default;
      void setText(const T& text) {
         *_text = text;
         publish(EventTextChanged(this)); //notify every view pointing at this model
      }
      void insertText(size_t index, const T& text) {
         _text->insert(index, text);
         publish(EventTextChanged(this));
      }
      T removeText(size_t index, size_t count){
         T erased = static_cast<T>(_text->substr(index, count));
         _text->erase(index, count);
         publish(EventTextChanged(this));
         return erased;
      }
      [[nodiscard]] const T& getText() const {return *_text;}
      [[nodiscard]] const std::shared_ptr<T>& textPtr() const {return _text;} //shared buffer, for aliasing
      void clear(){setText(T());}
   private:
      std::shared_ptr<T> _text;
   };

   // A Widget that renders a TextRenderModel. It does not own the text directly; it holds
   // a shared_ptr to the model and subscribes to it, so constructing one view from another
   // (the shared-model constructor) makes them mirror the same text and update together.
   class TextRenderView : public Widget {
   public:
      REYENGINE_OBJECT(TextRenderView)
      using Model = TextRenderModel<TrString>;
      //own a fresh model wrapping the given (possibly shared) text buffer
      explicit TextRenderView(std::shared_ptr<TrString> text)
      : _model(std::make_shared<Model>(std::move(text))) { bindModel(); }
      //point at an existing view's model: same buffer, same notifications
      explicit TextRenderView(const std::shared_ptr<TextRenderView>& other)
      : _model(other->_model) { bindModel(); }
      ~TextRenderView() override = default;

      [[nodiscard]] const TrString& getText() const {return _model->getText();}
      void setText(const TrString& text){_model->setText(text);}
      [[nodiscard]] const std::shared_ptr<Model>& getModel() const {return _model;}
   protected:
      void _init() override;
      void render2D(RenderContext&) const override;
      //called whenever the shared model's text changes (from this view or any other)
      virtual void _on_text_changed(){}
      std::shared_ptr<Model> _model;
   private:
      //subscribe to the shared model so model mutations reach this view's _on_text_changed
      void bindModel(){
         subscribe<Model::EventTextChanged>(_model, [this](const auto&){_on_text_changed();});
      }
   };
}
