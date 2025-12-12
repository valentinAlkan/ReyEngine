#pragma once
#include "Layout.h"
#include "Theme.h"

namespace ReyEngine{
   class TabContainer : public Layout {
   public:
      REYENGINE_OBJECT(TabContainer)
      EVENT_ARGS(EventTabShown, 10712812863217, Widget* widget)
      , widget(widget)
      {}
         Widget* widget;
      };
      EVENT_ARGS(EventTabHidden, 10712812863218, Widget* widget)
      , widget(widget)
         {}
      Widget* widget;
      };
      EVENT_ARGS(EventTabCreated, 10712812863219, Widget* widget)
      , widget(widget)
         {}
         Widget* widget;
      };
      EVENT_ARGS(EventTabRemoved, 10712812863220, Widget* widget)
      , widget(widget)
         {}
         Widget* widget;
      };
      TabContainer(): Layout(LayoutDir::OTHER){}
      void render2D() const override;
      void _on_child_added_to_tree(TypeNode*) override;
      void _on_child_removed_from_tree(TypeNode*) override;
      void setCurrentTab(Widget*);
      std::optional<Widget*> getCurrentTab(){return currentTab;}
   protected:
      Widget* _unhandled_input(const InputEvent&) override;
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