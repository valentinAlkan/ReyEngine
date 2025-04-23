#pragma once
#include "Control.h"
#include "Button.h"
#include "Layout.h"
#include "Label.h"

namespace ReyEngine {
   template <typename T>
   struct is_string_view_array : std::false_type {}; //false by default

   template <size_t N>
   struct is_string_view_array<std::array<std::string_view, N>> : std::true_type {}; //true whenever the type matches

   // Type trait for checking if T is a std::array<enum, N>
   template <typename T>
   struct is_enum_array : std::false_type {};

   template <typename E, size_t N>
   struct is_enum_array<std::array<E, N>> : std::integral_constant<bool, std::is_enum_v<E>> {};

   template <typename R, size_t N>
   class Dialog : public Control {
   public:
      REYENGINE_OBJECT(Dialog);
      constexpr Dialog(const std::array<std::string_view, N>& options, const std::array<R, N>& retvals, std::string_view text = "", Layout::LayoutDir layoutDirection = Layout::LayoutDir::HORIZONTAL)
      : options(options)
      , retvals(retvals)
      , text(text)
      , layoutDirection(layoutDirection)
      {}

   protected:
      void _init() override{
         setSize(1000, 1000); //todo: find minimum size
        //create buttonLayout
        auto buttonLayoutPair = make_node<Layout>("buttonLayout", layoutDirection);
        if(!text.empty()) {
           auto textPair = make_node<Label>("textLabel", std::string(text));
           if (layoutDirection == Layout::LayoutDir::HORIZONTAL) { //Create a vertical layout for the text on top first
              auto layoutPair = make_node<Layout>("textLayout", Layout::LayoutDir::VERTICAL);
              layoutPair.second->addChild(std::move(textPair.second));
           } else {
               buttonLayoutPair.second->addChild(std::move(textPair.second));
           }
        }

         for(int i = 0; i < options.size(); i++){
            auto optionPair= make_node<PushButton>(std::string(options[i]) + "Button");
            optionPair.first->setText(std::string(options[i]));
            buttonLayoutPair.second->addChild(std::move(optionPair.second));
         }

         //todo: add button callbacks
      getNode()->addChild(std::move(buttonLayoutPair.second));
      }

   private:
      std::array<std::string_view, N> options;
      std::array<R, N> retvals;
      Layout::LayoutDir layoutDirection;
      std::string_view text;
   };
}
