#pragma once
#include "Layout.h"

namespace ReyEngine{
   class TabTheme : public Style::Theme {
   public:
      TabTheme()
      : Style::Theme("DefaultTabTheme")
      , PROPERTY_DECLARE(tabMargin, 0)
      , PROPERTY_DECLARE(tabHeight, 20)
      , PROPERTY_DECLARE(tabWidth, 100)
      {}
      IntProperty tabMargin; //distance from left/right edge to first tab
      IntProperty tabHeight;
      IntProperty tabWidth;
   protected:
      void registerProperties() override{
         Style::Theme::registerProperties();
      }
   };

   class TabContainer : public Layout {
      REYENGINE_SERIALIZER(TabContainer, Layout)
      REYENGINE_DECLARE_STATIC_CONSTEXPR_TYPENAME(TabContainer)
      TabContainer(const std::string& instanceName): Layout(instanceName, TYPE_NAME, LayoutDir::OTHER)
      , NamedInstance(instanceName, TYPE_NAME)
      , PROPERTY_DECLARE(currentTab, 0){
         //replace generic style with tab style
         theme = std::make_shared<TabTheme>();
         tabTheme = std::static_pointer_cast<TabTheme>(theme); //so we dont have to bind to a temporary and can return a ref
      }
   public:
      void render() const override;
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