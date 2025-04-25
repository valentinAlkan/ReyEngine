#pragma once
#include "Control.h"
#include "Button.h"
#include "Layout.h"
#include "Label.h"

namespace ReyEngine {
   template <typename T, size_t N>
   class Dialog : public Control {
   public:
      REYENGINE_OBJECT(Dialog);
      Dialog(const std::array<std::pair<std::string_view, T>, N>& options, std::string_view text = "", Layout::LayoutDir layoutDirection = Layout::LayoutDir::HORIZONTAL)
      : options(options)
      , messageText(text)
      , layoutDirection(layoutDirection)
      {
         setSize(320, 240); //todo: find minimum size
      }
      EVENT_ARGS(DialogOpenEvent, 6454983, const Dialog& dialog)
      , dialog(dialog)
      {}
         const Dialog& dialog;
      };

      EVENT_ARGS(DialogCloseEvent, 6454984, std::string option, T value)
      , option(option)
      , value(value)
      {}
         const std::string option;
         const T value;
      };


   protected:
      void _init() override{
         //calculate splits
         std::vector<Percent> percents;
         for (int i=0; i<N; i++){
            percents.emplace_back(100/N);
         }
         //create manual layout since these things can't be resized (easily)
         std::vector<Rect<float>> rects;
         if(!messageText.empty()) {
            //split the main rect into smaller rects
            switch (layoutDirection){
               case Layout::LayoutDir::VERTICAL:
                  rects = getSizeRect().splitV(percents);
                  break;
               case Layout::LayoutDir::HORIZONTAL:
                  rects = getSizeRect().splitH(percents);
                  break;
            }
         }

         for(int i = 0; i < N; i++){
            //emsmallen the rects to create padding between them
            auto& btnRect = rects.at(i);
            btnRect.embiggen(-5);
            //cache rect center (since its about to change)
            auto center = btnRect.center();
            //cap their size
            auto height = measureText(std::string(options.at(i).first), theme->font).y;
            btnRect.setHeight(height + 5);
            //re-center the rects
            btnRect.centerOnPoint(center);

            auto [btn, node]= make_node<PushButton>(std::string(options.at(i).first) + "Button");
            btn->setText(std::string(options[i].first));
            btn->setRect(btnRect);

            //add metadata T
            static constexpr std::string_view METADATA_VALUE_NAME = "value";
            btn->setMetaData<T>(std::string(METADATA_VALUE_NAME), options.at(i).second);

            auto btnCB = [btn, this](const PushButton::ButtonPressEvent& event){
               auto publisher = event.publisher->as<Button>().value();
               auto option = publisher->getText();
               auto value = publisher->getMetaData<T>(std::string(METADATA_VALUE_NAME));
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
         }

         //add the label
         {
            auto [label, node] = make_node<Label>("MessageLabel", messageText);
            addChild(std::move(node));
            //todo: center correctly
         }

      }

   private:
      const std::array<std::pair<std::string_view, T>, N> options;
      const Layout::LayoutDir layoutDirection;
      const std::string messageText;
   };
}
