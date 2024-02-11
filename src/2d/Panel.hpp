#pragma once
#include <utility>

#include "Control.hpp"
#include "Layout.h"

// A simple visual panel that just gives us something to look at. No other functionality.
class Panel : public Control {
public:
   static constexpr std::string_view LAYOUT_NAME = "layout";
   static constexpr Layout::LayoutDir defaultLayout = Layout::LayoutDir::VERTICAL;
   REYENGINE_OBJECT(Panel, Control)
   , PROPERTY_DECLARE(_layoutDir, Layout::LayoutDir::HORIZONTAL)
   {
      theme->background.colorPrimary.set(ReyEngine::ColorRGBA(94, 142, 181, 255));
   }
public:
   void render() const override {
      auto roundness = theme->roundness.value;
      auto color = theme->background.colorPrimary.value;
      _drawRectangleRounded(_rect.value.toSizeRect(), roundness, 1, color);
   }
   void _init() override {}
   void _process(float dt) override {}
   void addToLayout(std::shared_ptr<BaseWidget> widget){
      std::shared_ptr<Layout> layout;
      auto layoutOpt = getLayout();
      if (layoutOpt){
         layout = layoutOpt.value();
      } else {
         switch (defaultLayout) {
            case Layout::LayoutDir::VERTICAL:
               layout = setLayout<VLayout>();
               break;
            case Layout::LayoutDir::HORIZONTAL:
               layout = setLayout<HLayout>();
               break;
         }
      }
      layout->addChild(std::move(widget));
   }
   template <typename T>
   std::shared_ptr<Layout> setLayout(){
      static_assert(std::is_base_of_v<Layout, T>);
      //create new layout
      auto newLayout = std::make_shared<T>(std::string(LAYOUT_NAME));
      //get existing layout
      auto layoutOpt = getChild(std::string(LAYOUT_NAME));
      if (layoutOpt) {
         auto layout = layoutOpt.value();
         //save grandchildren
         for (auto grandchild: layout->getChildren()) {
            //remove grandchild from old layout
            layout->removeChild(grandchild->getName());
            //add grandchild to new layout
            newLayout->addChild(grandchild);
         }
         //remove old layout
         removeChild(std::string(LAYOUT_NAME));
      }
      //add new layout
      addChild(newLayout);

      //keep track of new child so we don't have to wait for registration
      _layout = newLayout;

      auto resizeCB = [newLayout](const WidgetResizeEvent& event){
         newLayout->setSize(event.size);
      };

      //connect to resize signal
      newLayout->template subscribe<WidgetResizeEvent>(toEventPublisher(), resizeCB);
      return newLayout;
   }

   std::optional<std::shared_ptr<Layout>> getLayout(){
      if (!_layout.expired()){
         return std::dynamic_pointer_cast<Layout>(_layout.lock());
      }
      return std::nullopt;
   };

   void registerProperties() override{
      //register properties specific to your type here.
      registerProperty(_layoutDir);
   }
protected:
   Layout::LayoutProperty _layoutDir;
   std::weak_ptr<Layout> _layout;
};