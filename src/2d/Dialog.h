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
   protected:
      void _init() override{
         //calculate splits
         std::vector<Percent> percents;
         for (int i=0; i<N; i++){
            percents.emplace_back(100/N);
         }
         //create manual layout since these things can't be resized (easily)
         std::array<Rect<float>, N> rects;
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

         for(int i = 0; i < options.size(); i++){
            auto btnRect = rects.at(i);
            auto [btn, node]= make_node<PushButton>(std::string(options.at(i)) + "Button");
            btn->setText(std::string(options[i]));
            btn->setRect(btnRect);
            auto btnCB = [](const PushButton::ButtonPressEvent& event){

            };

            btn->subscribe<PushButton::ButtonPressEvent>(this, btnCB);
         }

      }

   private:
      const std::array<std::pair<std::string_view, T>, N> options;
      const Layout::LayoutDir layoutDirection;
      const std::string_view messageText;
   };
}
