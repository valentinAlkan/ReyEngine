#pragma once
#include "Layout.h"
#include "Theme.h"

namespace ReyEngine{
   class TabContainer : public Layout {
   public:
      REYENGINE_OBJECT(TabContainer)
      TabContainer(): Layout(LayoutDir::OTHER){}
      void render2D() const override;
      void _on_child_added_to_tree(TypeNode*) override;
      void setCurrentTab(Widget*);
      std::optional<Widget*> getCurrentTab(){return currentTab;}
   protected:
      Handled _unhandled_input(const InputEvent&) override;
      void arrangeChildren() override;
      std::optional<Widget*> currentTab;

      float tabMargin       = 2; //distance from left/right edge to first tab
      float tabHeight       = 22;
      float tabWidthMinimum = 40;
      float tabPadding      = 2;
   private:
      struct TabData{
         TabData(Widget* widget, const Rect<float>& rect)
         : widget(widget)
         , tabRect(rect)
         {}
         Widget* widget;
         Rect<float> tabRect;
      };
      ReyEngine::Rect<float> _childBoundingRect;
      std::vector<TabData> _tabs;
   };
}