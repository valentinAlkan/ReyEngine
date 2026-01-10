#pragma once
#include <utility>
#include "Widget.h"
#include "Layout.h"
#include "Label.h"
#include "Canvas.h"

// A simple visual panel that just gives us something to look at. No other functionality.
namespace ReyEngine{
#define USE_TYPE_ERROR_MSG "InstanceName must be a string or a string-like type (e.g., const char*, std::string). \
You might be trying to use make_child like its non-panel equivalent, but it doesn't work like that. \
Typically, with a panel, you add children to the viewable area, otherwise children could \
be placed on top of the banner or buttons, which isn't usually what we want. If you do want \
that functionality, use make_child_built_in. At any rate, neither function accepts a parent, \
because we infer who the parent is. So just delete the parent parameter. If you're trying to use the \
normal make_child function but cant' because the compiler is resolving it to this one, simply prepend those calls \
with :: like this-> ::make_child(...)"
   class Panel : public Canvas {
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

      ///we have to change the way make child and add child work since we don't actually want anyone
      /// add a child to the panel itself, typically. usually what we want is for the child to be added
      /// as a child of the viewable area.
      template<typename T, typename InstanceName, typename... Args>
      std::shared_ptr<T> make_child(InstanceName&& instanceName, Args&&... args){
         static_assert(std::is_convertible_v<InstanceName, std::string_view>, USE_TYPE_ERROR_MSG);
         return Internal::Tree::_make_child<T>(_viewArea->getNode(), instanceName, std::forward<Args>(args)...);
      }
      /// Creates a child as part of the panel widget's built-in functionality, not as a child
      /// of the viewable area
      template<typename T, typename InstanceName, typename... Args>
      std::shared_ptr<T> make_child_built_in(InstanceName&& instanceName, Args&&... args){
         static_assert(std::is_convertible_v<InstanceName, std::string_view>, USE_TYPE_ERROR_MSG);
         return Internal::Tree::_make_child<T>(getNode(), instanceName, std::forward<Args>(args)...);
      }
//      void render2DBegin() override;
//      void render2DEnd() override;
      void render2D() const override;
      void setTitle(const std::string& newtitle);
//      void addChildToPanel(std::shared_ptr<Widget> child);
      inline void setResizable(bool resizeable){_isResizable = resizeable;}
      inline bool getResizable(){return _isResizable;}
      void hide();
      void show();
   protected:
      void setVisible(bool visible){Widget::setVisible(visible);} //hide from public view
      Widget* _unhandled_input(const InputEvent&) override;
      void _on_mouse_exit() override;
      void __init() override;
      void _on_rect_changed() override;
//      ReyEngine::Rect<R_FLOAT> getScissorArea();
      enum class ResizeDir{NONE, N, E, S, W, NE, SE, SW, NW};

      std::string panelTitle;
      struct Header {
         Rect<float> rect;
         Rect<float> btnClose;
         Rect<float> btnMinimize;
      } _header;
      bool showHeader;

      Rect<float> titleLabel;
//      ReyEngine::Pos<R_FLOAT> offset;
      ReyEngine::Pos<R_FLOAT> dragStart;
//      ReyEngine::Rect<R_FLOAT> resizeStartRect;
//      ReyEngine::Rect<R_FLOAT> cacheRect; //for caching size before maximize
      bool _isDragable = true;
      enum class DragState {NONE, DRAGGING, CANT_DRAG};
      DragState _dragState;
      bool _isResizable = false;
      bool _isMinimized = false;
      bool _isMaximized = false;
      std::shared_ptr<ReyEngine::Widget> _viewArea; //any child added to the window will be moved to the placement widget

      InputFilter _filterCache;

      ResizeDir _resizeDir = ResizeDir::NONE;
      std::array<ReyEngine::Rect<R_FLOAT>, 4> stretchRegion; //top/right/bottom/left
      ReyEngine::Rect<R_FLOAT>& REGION_NORTH;
      ReyEngine::Rect<R_FLOAT>& REGION_EAST;
      ReyEngine::Rect<R_FLOAT>& REGION_WEST;
      ReyEngine::Rect<R_FLOAT>& REGION_SOUTH;
      ReyEngine::Pos<float> testPos;
   };
}
#undef PANEL_MAKE_CHILD_USE_TYPE_ERROR_MSG