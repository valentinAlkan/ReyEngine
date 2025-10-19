#pragma once
#include "Widget.h"

namespace ReyEngine {
   class Separator : public Widget {
   public:
      REYENGINE_OBJECT(Separator)
      enum class SeparatorStyle{VERTICAL, HORIZONTAL};
      Separator(SeparatorStyle style)
      : _separatorStyle(style)
      {}
      void setStyle(SeparatorStyle newStyle){_separatorStyle = newStyle; _calculateSize();}
      void setSeparation(float separation){_separation = separation; _calculateSize();}
      [[nodiscard]] SeparatorStyle getStyle() const {return _separatorStyle;}
      [[nodiscard]] float getSeparation() const {return _separation;}
   protected:
      static constexpr float DEFAULT_SEPARATION = 5;
      void render2D() const override {
         drawLine(_visual, theme->foreground.linethick, theme->foreground.colorPrimary);
      }
      void _init() override {
         _calculateSize();
      }
      void _calculateSize(){
         switch (_separatorStyle) {
            case SeparatorStyle::VERTICAL:
               setMaxWidth(DEFAULT_SEPARATION);
               setMaxHeight(std::numeric_limits<float>::max());
               break;
            case SeparatorStyle::HORIZONTAL:
               setMaxHeight(DEFAULT_SEPARATION);
               setMaxWidth(std::numeric_limits<float>::max());
               break;
         }
      }
      void _on_rect_changed() override {
         switch(_separatorStyle) {
            case SeparatorStyle::VERTICAL:{
               auto sizeRect = getSizeRect();
               _visual = Line<float>(sizeRect.top().midpoint(), sizeRect.bottom().midpoint());
               break;}
               case SeparatorStyle::HORIZONTAL:{
                  auto sizeRect = getSizeRect();
                  _visual = Line<float>(sizeRect.left().midpoint(), sizeRect.right().midpoint());
                  break;}
         }
      }
      SeparatorStyle _separatorStyle;
      float _separation = 5;
      Line<float> _visual;
   };
}

