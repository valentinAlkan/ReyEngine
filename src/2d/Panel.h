#pragma once
#include <utility>

#include "Control.h"
#include "Layout.h"

// A simple visual panel that just gives us something to look at. No other functionality.
class Panel : public BaseWidget {
public:
   static constexpr char VLAYOUT_NAME[] = "__vlayout";
   static constexpr char MENU_NAME[] = "__menu";
   static constexpr char WINDOW_NAME[] = "__window";
   static constexpr char BTN_CLOSE_NAME[] = "__btn_close";
   REYENGINE_OBJECT(Panel, BaseWidget)
   , PROPERTY_DECLARE(panelTitle)
   , PROPERTY_DECLARE(showHeader, true)
   , PROPERTY_DECLARE(_isResizable)
   {
      theme->background = Style::Fill::SOLID;
      theme->background.colorPrimary = ReyEngine::ColorRGBA(94, 142, 181, 255);
   }
public:
   void _init() override;
   void _on_rect_changed() override;
   Handled _unhandled_input(const InputEvent&, const std::optional<UnhandledMouseInput>&) override;
   void renderBegin(ReyEngine::Pos<double>& textureOffset) override;
   void renderEnd() override;
   void render() const override;
   void registerProperties() override;
   std::optional<std::shared_ptr<BaseWidget>> addChildInternal(std::shared_ptr<BaseWidget> child){return BaseWidget::addChild(child);}
   std::optional<std::shared_ptr<BaseWidget>> addChildToPanel(std::shared_ptr<BaseWidget> child);
   inline void setResizable(bool resizeable){_isResizable = resizeable;}
   inline bool getResizable(){return _isResizable;}

protected:
   enum class ResizeDir{NONE, N, E, S, W, NE, SE, SW, NW};

   std::optional<std::shared_ptr<BaseWidget>> addChild(std::shared_ptr<BaseWidget> child) override{return BaseWidget::addChild(child);}
   StringProperty panelTitle;
   BoolProperty showHeader;
   std::shared_ptr<VLayout> vlayout;
   std::shared_ptr<HLayout> menuBar;
   std::shared_ptr<Control> window;
   ReyEngine::Pos<int> offset;
   ReyEngine::Pos<int> mousePos;
   ReyEngine::Pos<int> dragStart;
   ReyEngine::Rect<int> startRect;
   bool _isDragging = false;
   BoolProperty _isResizable;
   ResizeDir _resizeDir = ResizeDir::NONE;
   std::array<ReyEngine::Rect<int>, 4> stretchRegion; //top/right/bottom/left
   ReyEngine::ScissorTarget _scissorTarget;
};