#pragma once
#include "Layout.h"

namespace ReyEngine{
   class TabTheme : public Style::Theme {
   public:
      TabTheme()
      : Style::Theme("DefaultTabTheme")
      , PROPERTY_DECLARE(tabMargin, 0)
      , PROPERTY_DECLARE(tabHeight, 20)
      , PROPERTY_DECLARE(tabWidthMinimum, 100)
      , PROPERTY_DECLARE(tabPadding, 3)
      {}
      IntProperty tabMargin; //distance from left/right edge to first tab
      IntProperty tabHeight;
      IntProperty tabWidthMinimum;
      IntProperty tabPadding;
   protected:
      void registerProperties() override{
         Style::Theme::registerProperties();
      }
   };

   class TabContainer : public Layout {
      REYENGINE_OBJECT_BUILD_ONLY_ARGS(TabContainer, Layout, BaseWidget, LayoutDir::OTHER)
      , PROPERTY_DECLARE(currentTab, 0){
         //replace generic style with tab style
         theme = std::make_shared<TabTheme>();
         tabTheme = std::static_pointer_cast<TabTheme>(theme); //so we dont have to bind to a temporary and can return a ref
      }
   public:
      REYENGINE_DEFAULT_BUILD(TabContainer)
      void render2D() const override;
      void registerProperties() override{};
      void _on_child_added(std::shared_ptr<BaseWidget>&) override;
      std::shared_ptr<TabTheme>& getTabTheme(){return tabTheme;}
      void setCurrentTab(int index);
      int getCurrentTab(){return currentTab;}
   protected:
      Handled _unhandled_input(const InputEvent&, const std::optional<UnhandledMouseInput>&) override;
      void arrangeChildren() override;
      IntProperty currentTab;
   private:
      std::shared_ptr<TabTheme> tabTheme;
      ReyEngine::Rect<int> _childBoundingRect;
      std::vector<ReyEngine::Rect<int>> _tabRects;
   };
}