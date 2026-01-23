#pragma once
#include <utility>
#include "Widget.h"
#include "Layout.h"
#include "Label.h"
#include "Canvas.h"

// A simple visual panel that just gives us something to look at. No other functionality.
namespace ReyEngine{
   class Panel : public Widget {
   public:
      REYENGINE_OBJECT(Panel)
      Panel(const std::string& panelTitle="");
   public:
      void render2D() const override;
      void setTitle(const std::string& newtitle);
      inline void setResizable(bool resizeable){_isResizable = resizeable;}
      inline bool getResizable(){return _isResizable;}
      void hide();
      void show();
   protected:
      void setVisible(bool visible){Widget::setVisible(visible);} //hide from public view
      Widget* _unhandled_input(const InputEvent&) override;
      void _on_mouse_exit() override;
      void _init() override;
      void _on_rect_changed() override;
      virtual void _on_about_to_show(){};
      virtual void _on_about_to_hide(){};
//      ReyEngine::Rect<R_FLOAT> getScissorArea();
      enum class ResizeDir{NONE, N, E, S, W, NE, SE, SW, NW};

      std::string _title;
      struct Header {
         Rect<float> rect;
         Rect<float> btnClose;
         Pos<float> titlePos;
         Rect<float> btnMinimize;
      } _header;
      bool showHeader;

//      Rect<float> titleLabel;
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
      bool _closeDown = false; //whether or not the close button is down

      InputFilter _filterCache;

      ResizeDir _resizeDir = ResizeDir::NONE;
      std::array<ReyEngine::Rect<R_FLOAT>, 4> stretchRegion; //top/right/bottom/left
      ReyEngine::Rect<R_FLOAT>& REGION_NORTH;
      ReyEngine::Rect<R_FLOAT>& REGION_EAST;
      ReyEngine::Rect<R_FLOAT>& REGION_WEST;
      ReyEngine::Rect<R_FLOAT>& REGION_SOUTH;
   };
}