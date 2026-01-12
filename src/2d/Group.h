#pragma once
#include "Widget.h"

namespace ReyEngine {
   class Group : public Layout {
   public:
      REYENGINE_OBJECT(Group)
      Group(const std::string& groupName)
      : Layout(LayoutDir::VERTICAL)
      , _groupName(groupName)
      {}
      void render2D() const override {
         drawLine(_frameRect.left(), 1.0, theme->foreground.colorPrimary);
         drawLine(_frameRect.right(), 1.0, theme->foreground.colorPrimary);
         drawLine(_frameRect.bottom(), 1.0, theme->foreground.colorPrimary);
         drawLine(_leftStub, 1.0, theme->foreground.colorPrimary);
         drawLine(_rightStub, 1.0, theme->foreground.colorPrimary);
         drawText(_groupName, _textPos, theme->font);
      }
      void setGroupName(const std::string& groupName){_groupName = groupName;}
      [[nodiscard]] std::string getGroupName() const {return _groupName;}
      void _on_rect_changed() override {
         static constexpr float STUB_OFFSET = 15;
         auto textHeight = measureText(_groupName, theme->font).y;
         _frameRect = getSizeRect();
         _frameRect = _frameRect.splitAtVPos(textHeight - textHeight * .5 ).second;
         _leftStub = _frameRect.top().shorten(_frameRect.width - STUB_OFFSET);
         _textPos = Pos<float>(20, 0);
         _rightStub = _frameRect.top().reverse().shorten(STUB_OFFSET * 2 + measureText(_groupName, theme->font).x);
         auto layoutLine = measureText(_groupName, theme->font).y;
         _layoutArea = getSizeRect().splitAtVPos(layoutLine).second.embiggen(-_extraMargin);
         arrangeChildren();
      }
   protected:
      float _extraMargin = 10;
      std::string _groupName;
      Rect<float> _groupRect; //the rect that contains all children
      Rect<float> _frameRect; //the visual frame detail
      Line<float> _leftStub;
      Line<float> _rightStub;
      Pos<float> _textPos;
   };
}