#pragma once
#include "Widget.h"

namespace ReyEngine {
   // An HSV color picker: a saturation/value field alongside a hue strip, an optional alpha
   // strip, and a preview swatch. Publishes EventColorChanged continuously while the user
   // drags and EventColorPicked when the mouse is released.
   class ColorPicker : public Widget {
   public:
      REYENGINE_OBJECT(ColorPicker)
      EVENT_ARGS(EventColorChanged, 561234987001, const ColorRGBA& color)
      , color(color)
      {}
         const ColorRGBA color;
      };

      EVENT_ARGS(EventColorPicked, 561234987002, const ColorRGBA& color)
      , color(color)
      {}
         const ColorRGBA color;
      };

      ColorPicker(const ColorRGBA& startColor = Colors::red){
         setMinSize(120, 120);
         setSize(220, 240);
         setColor(startColor, false);
      }

      [[nodiscard]] ColorRGBA getColor() const {return fromHSV(_hue, _sat, _val, _alpha);}
      void setColor(const ColorRGBA& color, bool publish = true);
      void setAlphaEnabled(bool enabled){_alphaEnabled = enabled; _compute_appearance();}
      [[nodiscard]] bool getAlphaEnabled() const {return _alphaEnabled;}
      void setReadoutEnabled(bool enabled){_readoutEnabled = enabled; _compute_appearance();}
      [[nodiscard]] bool getReadoutEnabled() const {return _readoutEnabled;}

      //hue is [0,360), saturation/value/alpha are [0,1]
      static ColorRGBA fromHSV(float hue, float sat, float val, float alpha = 1.0f);
      //when the color is achromatic (sat == 0), hue is left unchanged
      static void toHSV(const ColorRGBA& color, float& hue, float& sat, float& val, float& alpha);
   protected:
      Handled _unhandled_input(const InputEvent& e) override;
      void render2D(RenderContext&) const override;
      void _on_rect_changed() override {_compute_appearance();}
      void _init() override {_compute_appearance();} //recompute once the theme (and thus font metrics) exists
   private:
      enum class DragTarget{NONE, SV, HUE, ALPHA};
      void _compute_appearance();
      void _apply_drag(const Pos<float>& localPos);

      float _hue = 0;   //[0,360)
      float _sat = 1;   //[0,1]
      float _val = 1;   //[0,1]
      float _alpha = 1; //[0,1]
      bool _alphaEnabled = true;
      bool _readoutEnabled = true;
      DragTarget _dragTarget = DragTarget::NONE;

      static constexpr float PAD = 4;
      static constexpr float STRIP_WIDTH = 18;
      static constexpr float SWATCH_HEIGHT = 22;
      static constexpr float SWATCH_PREVIEW_WIDTH = 48; //swatch width when the readout sits beside it
      float _readoutLineHeight = 14;
      Rect<float> _svField = {0, 0, 0, 0};  //saturation (x) / value (y) picking area
      Rect<float> _hueBar = {0, 0, 0, 0};   //vertical hue strip
      Rect<float> _alphaBar = {0, 0, 0, 0}; //vertical alpha strip
      Rect<float> _swatch = {0, 0, 0, 0};   //preview of the current color
      Rect<float> _readout = {0, 0, 0, 0};  //text readout of the current color in various formats
   };
}
