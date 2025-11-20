#pragma once
#include "Control.h"
#include "Button.h"
#include "Layout.h"
#include "Label.h"

namespace ReyEngine {
   enum class DialogOptions{CANCEL, OK, CLOSE, YES, NO};

   namespace __ {
      constexpr auto toString(DialogOptions option){
         switch (option) {
            case DialogOptions::OK: return "Ok";
            case DialogOptions::CANCEL: return "Cancel";
            case DialogOptions::CLOSE: return "Close";
            case DialogOptions::NO: return "No";
            case DialogOptions::YES: return "Yes";
            default: return "";
         }
      }
   }

   template <size_t N, typename OptMetaType=void>
   class Dialog : public Control {
   public:
      REYENGINE_OBJECT(Dialog);
      Dialog(std::array<DialogOptions, N> options, const std::string& message="", Layout::LayoutDir layoutDirection = Layout::LayoutDir::HORIZONTAL)
      : options(options)
      , message(message)
      , layoutDirection(layoutDirection)
      {
         if (layoutDirection != Layout::LayoutDir::HORIZONTAL && layoutDirection != Layout::LayoutDir::VERTICAL) {
            Logger::error() << "Dialogs do not support this layout direction. Defaulting to Horizontal" << std::endl;
         }
         setSize(320, 240); //todo: find minimum size
         _visible = false; //no events
      }

      EVENT_ARGS(DialogOpenEvent, 6454983, const Dialog& dialog)
         , dialog(dialog)
         {}
         const Dialog& dialog;
      };

      EVENT_ARGS(DialogCloseEvent, 6454984, const std::string& option, DialogOptions value)
         , value(value)
         {}
         const DialogOptions value;
      };

      void show(){ setVisible(true);} //setVisible calls onvis change
      void hide(){ setVisible(false);} //setVisible calls onvis change
      void setMessage(const std::string& m){message = m;}

   protected:
      void _init() override{
         //calculate splits
         std::vector<Percent> percents;
         for (size_t i=0; i<N; i++){
            percents.emplace_back(100/N);
         }

         //create manual layout since these things can't be resized (easily)
         std::vector<Rect<float>> rects;
         //split the main rect into smaller rects
         switch (layoutDirection){
            case Layout::LayoutDir::VERTICAL:
               rects = getSizeRect().splitV(percents);
               break;
            default:
            case Layout::LayoutDir::HORIZONTAL:
               rects = getSizeRect().splitH(percents);
               break;
         }

         size_t i = 0;
         for(const auto& option : options){
            const auto& text = __::toString(option);
            //emsmallen the rects to create padding between them
            auto& btnRect = rects.at(i);
            btnRect.embiggen(-5);
            //cache rect center (since its about to change)
            auto center = btnRect.center();
            //cap their size
            auto height = measureText(std::string(text), theme->font).y;
            btnRect.setHeight(height + 5);
            //re-center the rects
            btnRect.centerOnPoint(center);
            auto [btn, node]= make_node<PushButton>(std::string(text) + "Button");
            btn->setText(std::string(text));
            btn->setRect(btnRect);
            //add metadata T
            static constexpr std::string_view METADATA_VALUE_NAME = "value";
            btn->setMetaData<DialogOptions>(std::string(METADATA_VALUE_NAME), option);
            auto btnCB = [btn, this](const PushButton::ButtonPressEvent& event){
               auto publisher = event.publisher->as<Button>().value();
               auto option = publisher->getText();
               auto value = publisher->getMetaData<DialogOptions>(std::string(METADATA_VALUE_NAME));
               setVisible(false);
               if (!value) {
                  Logger::error() << "Invalid metadata for dialog option " << option << std::endl;
                  return;
               }
               DialogCloseEvent closeEvent(this, option, value.value());
               publish(closeEvent);
            };
            subscribe<PushButton::ButtonPressEvent>(btn, btnCB);
            addChild(std::move(node));
            ++i;
         }

         //add the label
         {
            auto [label, node] = make_node<Label>("MessageLabel", message);
            addChild(std::move(node));
            //todo: center correctly
         }
      }

      void _on_visibility_changed() override {
         if (_visible) {
            DialogOpenEvent event(this, *this);
            publish(event);
         }
      }

   private:
      const std::array<DialogOptions, N> options;
      const Layout::LayoutDir layoutDirection;
      std::string message;
   };
}
