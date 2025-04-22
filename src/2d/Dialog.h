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

   template <typename Options, typename Enums>
   class Dialog : public Control {
   public:
      REYENGINE_OBJECT(Dialog);
      Dialog(Options options, Enums enums, Layout::LayoutDir layoutDirection = Layout::LayoutDir::HORIZONTAL, std::string_view text = "")
      : options(options)
      , enums(enums)
      , layoutDirection(layoutDirection)
      , text(text)
      {
         static_assert(is_string_view_array<Options>::value, "Options must be a std::array<std::string_view, N>");
         static_assert(is_enum_array<Enums>::value, "Enums must be a std::array<Enum, N>");

         static_assert(options.size() == enums.size(), "options and enums must have the same number of elements");
      }

   protected:
      void _init() override{
        //create buttonLayout
        auto buttonLayoutPair = make_node<Layout>("buttonLayout", layoutDirection);
        if(!text.empty()) {
           auto textPair = make_node<Label>("textLabel", text);
           if (layoutDirection == Layout::LayoutDir::HORIZONTAL) { //Create a vertical layout for the text on top first
              auto layoutPair = make_node<Layout>("textLayout", Layout::LayoutDir::VERTICAL);
              layoutPair.second->addChild(std::move(textPair.second));
           } else {
               buttonLayoutPair.second->addChild(std::move(textPair.second));
           }
        }

         for(int i = 0; i < options.size(); i++){
            auto optionPair= make_node<PushButton>(options[i] + "Button");
            optionPair.first->setText(options[i]);
            buttonLayoutPair.second->addChild(optionPair);
         }
      getNode()->addChild(std::move(buttonLayoutPair.second));
      }

   private:
      Options options;
      Enums enums;
      Layout::LayoutDir layoutDirection;
      std::string_view text;
   };
}
