#pragma once
#include <utility>
#include "Widget.h"
#include "Layout.h"
#include "Label.h"

// A simple visual panel that just gives us something to look at. No other functionality.
namespace ReyEngine{
   class Panel : public Widget {
   public:
      REYENGINE_OBJECT(Panel)
      static constexpr char VLAYOUT_NAME[] = "__vlayout";
      static constexpr char MENU_NAME[] = "__menu";
      static constexpr char WINDOW_NAME[] = "__window";
      static constexpr char BTN_CLOSE_NAME[] = "__btn_close";
      static constexpr char BTN_MIN_NAME[] = "__btn_min";
      static constexpr char BTN_MAX_NAME[] = "__btn_max";
      static constexpr char TITLE_LABEL_NAME[] = "__titleLabel";
      Panel();
   public:
      TypeNode* addChild(std::unique_ptr<TypeNode>&& child);
      template<typename T, typename InstanceName, typename... Args>
      std::shared_ptr<T> make_child(InstanceName&& instanceName, Args&&... args){
         return Internal::Tree::_make_child<T>(getNode(), instanceName, std::forward<Args>(args)...);
      }
      /// Creates a child as part of the panel widget's built-in functionality, not as a child
      /// of the viewable area
      template<typename T, typename InstanceName, typename... Args>
      std::shared_ptr<T> make_child_built_in(InstanceName&& instanceName, Args&&... args){
         return Internal::Tree::_make_child<T>(getNode(), instanceName, std::forward<Args>(args)...);
      }
      void render2DBegin() override;
      void render2DEnd() override;
      void render2D() const override;
      void setTitle(const std::string& newtitle);
//      void addChildToPanel(std::shared_ptr<Widget> child);
      inline void setResizable(bool resizeable){_isResizable = resizeable;}
      inline bool getResizable(){return _isResizable;}

   protected:
      Widget* _unhandled_input(const InputEvent&) override;
      void _on_mouse_exit() override;
      void _init() override;
      void _on_rect_changed() override;
      ReyEngine::Rect<R_FLOAT> getScissorArea();
      enum class ResizeDir{NONE, N, E, S, W, NE, SE, SW, NW};

      std::string panelTitle;
      bool showHeader;
      struct MenuBar {
         Rect<float> bar;
         std::shared_ptr<Layout> btnCluster;
      };

      MenuBar menuBar;
      Rect<float> titleLabel;
      ReyEngine::Pos<R_FLOAT> offset;
      ReyEngine::Pos<R_FLOAT> dragStart;
      ReyEngine::Rect<R_FLOAT> resizeStartRect;
      ReyEngine::Rect<R_FLOAT> cacheRect; //for caching size before maximize
      bool _isDragging = false;
      bool _isResizable = true;
      bool _isMinimized = false;
      bool _isMaximized = false;
      std::shared_ptr<ReyEngine::Control> _viewArea; //any child added to the window will be moved to the placement widget

      InputFilter _filterCache;

      ResizeDir _resizeDir = ResizeDir::NONE;
      std::array<ReyEngine::Rect<R_FLOAT>, 4> stretchRegion; //top/right/bottom/left
      ReyEngine::Rect<R_FLOAT>& REGION_NORTH;
      ReyEngine::Rect<R_FLOAT>& REGION_EAST;
      ReyEngine::Rect<R_FLOAT>& REGION_WEST;
      ReyEngine::Rect<R_FLOAT>& REGION_SOUTH;
      ReyEngine::Rect<R_FLOAT> _scissorArea;
   };
}