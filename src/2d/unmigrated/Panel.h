#pragma once
#include <utility>
#include "Control.h"
#include "Layout.h"
#include "Label.h"

// A simple visual panel that just gives us something to look at. No other functionality.
namespace ReyEngine{
   class Panel : public BaseWidget {
   public:
      static constexpr char VLAYOUT_NAME[] = "__vlayout";
      static constexpr char MENU_NAME[] = "__menu";
      static constexpr char WINDOW_NAME[] = "__window";
      static constexpr char BTN_CLOSE_NAME[] = "__btn_close";
      static constexpr char BTN_MIN_NAME[] = "__btn_min";
      static constexpr char BTN_MAX_NAME[] = "__btn_max";
      static constexpr char TITLE_LABEL_NAME[] = "__titleLabel";
      REYENGINE_OBJECT_BUILD_ONLY(Panel, BaseWidget, BaseWidget)
      , PROPERTY_DECLARE(panelTitle)
      , PROPERTY_DECLARE(showHeader, true)
      , PROPERTY_DECLARE(_isResizable, true)
      , PROPERTY_DECLARE(_isMinimized)
      , PROPERTY_DECLARE(_isMaximized)
      {
         theme->background = Style::Fill::SOLID;
         theme->background.colorPrimary = ReyEngine::ColorRGBA(94, 142, 181, 255);
      }
   public:
      REYENGINE_DEFAULT_BUILD(Panel)
      void _init() override;
      void _on_rect_changed() override;
      Handled _unhandled_input(const InputEvent&, const std::optional<UnhandledMouseInput>&) override;
      void addChild(std::shared_ptr<BaseWidget> child) = delete;
      void render2DBegin() override;
      void render2DEnd() override;
      void render2D() const override;
      void registerProperties() override;
      void addChildInternal(std::shared_ptr<BaseWidget> child){BaseWidget::addChild(child);}
      void setTitle(const std::string& newtitle);
      void addChildToPanel(std::shared_ptr<BaseWidget> child);
      inline void setResizable(bool resizeable){_isResizable = resizeable;}
      inline bool getResizable(){return _isResizable;}

   protected:
      void _on_mouse_exit() override;
      ReyEngine::Rect<R_FLOAT> getScissorArea();
      enum class ResizeDir{NONE, N, E, S, W, NE, SE, SW, NW};

      StringProperty panelTitle;
      BoolProperty showHeader;
      std::shared_ptr<VLayout> vlayout;
      std::shared_ptr<HLayout> menuBar;
      std::shared_ptr<Control> window;
      std::shared_ptr<Label> titleLabel;
      ReyEngine::Pos<R_FLOAT> offset;
      ReyEngine::Pos<int> mousePos;
      ReyEngine::Pos<int> dragStart;
      ReyEngine::Rect<R_FLOAT> resizeStartRect;
      ReyEngine::Rect<R_FLOAT> cacheRect; //for caching size before maximize
      bool _isDragging = false;
      BoolProperty _isResizable;
      BoolProperty _isMinimized;
      BoolProperty _isMaximized;

      InputFilter _filterCache;

      ResizeDir _resizeDir = ResizeDir::NONE;
      std::array<ReyEngine::Rect<R_FLOAT>, 4> stretchRegion; //top/right/bottom/left
      ReyEngine::Rect<R_FLOAT> _scissorArea;
   };
}