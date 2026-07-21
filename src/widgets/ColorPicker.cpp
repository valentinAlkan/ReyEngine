#include "ColorPicker.h"
#include <algorithm>
#include <cmath>
#include <cstdio>

using namespace std;
using namespace ReyEngine;

/////////////////////////////////////////////////////////////////////////////////////////
ColorRGBA ColorPicker::fromHSV(float hue, float sat, float val, float alpha) {
   hue = fmod(hue, 360.0f);
   if (hue < 0) hue += 360.0f;
   const float c = val * sat;
   const float hp = hue / 60.0f;
   const float x = c * (1.0f - fabs(fmod(hp, 2.0f) - 1.0f));
   float r = 0, g = 0, b = 0;
   switch (static_cast<int>(hp)) {
      case 0: r = c; g = x; break;
      case 1: r = x; g = c; break;
      case 2: g = c; b = x; break;
      case 3: g = x; b = c; break;
      case 4: r = x; b = c; break;
      default: r = c; b = x; break;
   }
   const float m = val - c;
   return {static_cast<int>(lround((r + m) * 255.0f)),
           static_cast<int>(lround((g + m) * 255.0f)),
           static_cast<int>(lround((b + m) * 255.0f)),
           static_cast<int>(lround(alpha * 255.0f))};
}

/////////////////////////////////////////////////////////////////////////////////////////
void ColorPicker::toHSV(const ColorRGBA& color, float& hue, float& sat, float& val, float& alpha) {
   const float r = color.r / 255.0f;
   const float g = color.g / 255.0f;
   const float b = color.b / 255.0f;
   const float maxc = max({r, g, b});
   const float minc = min({r, g, b});
   const float delta = maxc - minc;
   val = maxc;
   sat = maxc > 0 ? delta / maxc : 0;
   alpha = color.a / 255.0f;
   if (delta > 0) {
      if (maxc == r)      hue = 60.0f * fmod((g - b) / delta, 6.0f);
      else if (maxc == g) hue = 60.0f * ((b - r) / delta + 2.0f);
      else                hue = 60.0f * ((r - g) / delta + 4.0f);
      if (hue < 0) hue += 360.0f;
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
void ColorPicker::setColor(const ColorRGBA& color, bool publishEvent) {
   toHSV(color, _hue, _sat, _val, _alpha);
   if (publishEvent) {
      EventColorChanged event(this, getColor());
      publish(event);
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
void ColorPicker::_compute_appearance() {
   const auto area = getSizeRect().embiggen(-PAD);
   if (theme && theme->font) _readoutLineHeight = theme->font->measure(" ").y;
   const float bottomHeight = _readoutEnabled ? max(SWATCH_HEIGHT, 3 * _readoutLineHeight) : SWATCH_HEIGHT;
   const float stripCount = _alphaEnabled ? 2 : 1;
   const float contentHeight = max(0.0f, area.height - bottomHeight - PAD);
   _svField = {area.x, area.y, max(0.0f, area.width - stripCount * (STRIP_WIDTH + PAD)), contentHeight};
   _hueBar = {_svField.x + _svField.width + PAD, area.y, STRIP_WIDTH, contentHeight};
   _alphaBar = _alphaEnabled ? Rect<float>{_hueBar.x + STRIP_WIDTH + PAD, area.y, STRIP_WIDTH, contentHeight} : Rect<float>{0, 0, 0, 0};
   const float bottomTop = area.y + area.height - bottomHeight;
   if (_readoutEnabled) {
      _swatch = {area.x, bottomTop, SWATCH_PREVIEW_WIDTH, bottomHeight};
      _readout = {_swatch.x + _swatch.width + PAD, bottomTop, max(0.0f, area.width - SWATCH_PREVIEW_WIDTH - PAD), bottomHeight};
   } else {
      _swatch = {area.x, bottomTop, area.width, bottomHeight};
      _readout = {0, 0, 0, 0};
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
void ColorPicker::_apply_drag(const Pos<float>& localPos) {
   auto fraction = [](float v, float lo, float len) {
      return len > 0 ? clamp((v - lo) / len, 0.0f, 1.0f) : 0.0f;
   };
   switch (_dragTarget) {
      case DragTarget::SV:
         _sat = fraction(localPos.x, _svField.x, _svField.width);
         _val = 1.0f - fraction(localPos.y, _svField.y, _svField.height);
         break;
      case DragTarget::HUE:
         _hue = 360.0f * fraction(localPos.y, _hueBar.y, _hueBar.height);
         break;
      case DragTarget::ALPHA:
         _alpha = 1.0f - fraction(localPos.y, _alphaBar.y, _alphaBar.height);
         break;
      case DragTarget::NONE:
         return;
   }
   EventColorChanged event(this, getColor());
   publish(event);
}

/////////////////////////////////////////////////////////////////////////////////////////
Handled ColorPicker::_unhandled_input(const InputEvent& e) {
   auto mouseEvent = e.isMouse();
   if (!mouseEvent) return nullptr;
   const auto localPos = mouseEvent.value()->getLocalPos();

   if (e.isEvent<InputEventMouseMotion>() && _dragTarget != DragTarget::NONE) {
      _apply_drag(localPos);
      return this;
   }

   if (e.isEvent<InputEventMouseButton>()) {
      auto& buttonEvent = e.toEvent<InputEventMouseButton>();
      if (buttonEvent.isDown && mouseEvent.value()->isInside()) {
         if (_svField.contains(localPos))                        _dragTarget = DragTarget::SV;
         else if (_hueBar.contains(localPos))                    _dragTarget = DragTarget::HUE;
         else if (_alphaEnabled && _alphaBar.contains(localPos)) _dragTarget = DragTarget::ALPHA;
         else return nullptr;
         setFocused(true);
         _apply_drag(localPos);
         return this;
      }
      if (!buttonEvent.isDown && _dragTarget != DragTarget::NONE) {
         _dragTarget = DragTarget::NONE;
         setFocused(false);
         EventColorPicked event(this, getColor());
         publish(event);
         return this;
      }
   }
   return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////
static void drawCheckerboard(const Rect<float>& area, float cellSize) {
   bool columnToggle = false;
   for (float x = area.x; x < area.x + area.width; x += cellSize) {
      const float w = min(cellSize, area.x + area.width - x);
      bool toggle = columnToggle;
      for (float y = area.y; y < area.y + area.height; y += cellSize) {
         const float h = min(cellSize, area.y + area.height - y);
         drawRectangle({x, y, w, h}, toggle ? Colors::lightGray : Colors::white);
         toggle = !toggle;
      }
      columnToggle = !columnToggle;
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
void ColorPicker::render2D(RenderContext&) const {
   //saturation/value field: white->hue horizontally, then fade to black vertically
   drawRectangleGradientH(_svField, Colors::white, fromHSV(_hue, 1, 1));
   drawRectangleGradientV(_svField, ColorRGBA(0, 0, 0, 0), ColorRGBA(0, 0, 0, 255));
   drawRectangleLines(_svField, 1.0, Colors::black);
   const Pos<float> svCursor = {_svField.x + _sat * _svField.width, _svField.y + (1.0f - _val) * _svField.height};
   drawCircleLines(Circle(svCursor, 5), _val > 0.5f ? Colors::black : Colors::white);

   //hue strip: six vertical gradient segments through the color wheel
   static constexpr ColorRGBA HUE_STOPS[] = {
      {255, 0, 0, 255}, {255, 255, 0, 255}, {0, 255, 0, 255},
      {0, 255, 255, 255}, {0, 0, 255, 255}, {255, 0, 255, 255}, {255, 0, 0, 255}};
   const float segmentHeight = _hueBar.height / 6.0f;
   for (int i = 0; i < 6; i++) {
      drawRectangleGradientV({_hueBar.x, _hueBar.y + i * segmentHeight, _hueBar.width, segmentHeight}, HUE_STOPS[i], HUE_STOPS[i + 1]);
   }
   drawRectangleLines(_hueBar, 1.0, Colors::black);
   const float hueY = _hueBar.y + (_hue / 360.0f) * _hueBar.height;
   drawRectangleLines({_hueBar.x - 1, hueY - 2, _hueBar.width + 2, 4}, 1.0, Colors::black);

   //alpha strip: current color fading to transparent over a checkerboard
   if (_alphaEnabled) {
      drawCheckerboard(_alphaBar, 6);
      const auto opaque = fromHSV(_hue, _sat, _val);
      drawRectangleGradientV(_alphaBar, opaque, ColorRGBA(opaque.r, opaque.g, opaque.b, 0));
      drawRectangleLines(_alphaBar, 1.0, Colors::black);
      const float alphaY = _alphaBar.y + (1.0f - _alpha) * _alphaBar.height;
      drawRectangleLines({_alphaBar.x - 1, alphaY - 2, _alphaBar.width + 2, 4}, 1.0, Colors::black);
   }

   //preview swatch
   drawCheckerboard(_swatch, 6);
   drawRectangle(_swatch, getColor());
   drawRectangleLines(_swatch, 1.0, Colors::black);

   //raw value readout
   if (_readoutEnabled && theme && theme->font) {
      const auto c = getColor();
      char line[48];
      float y = _readout.y;
      if (_alphaEnabled) snprintf(line, sizeof(line), "HEX  #%02X%02X%02X%02X", c.r, c.g, c.b, c.a);
      else               snprintf(line, sizeof(line), "HEX  #%02X%02X%02X", c.r, c.g, c.b);
      drawText(line, {_readout.x, y}, theme->font);
      y += _readoutLineHeight;
      if (_alphaEnabled) snprintf(line, sizeof(line), "RGBA %d, %d, %d, %d", c.r, c.g, c.b, c.a);
      else               snprintf(line, sizeof(line), "RGB  %d, %d, %d", c.r, c.g, c.b);
      drawText(line, {_readout.x, y}, theme->font);
      y += _readoutLineHeight;
      snprintf(line, sizeof(line), "HSV  %d, %d%%, %d%%", static_cast<int>(lround(_hue)), static_cast<int>(lround(_sat * 100)), static_cast<int>(lround(_val * 100)));
      drawText(line, {_readout.x, y}, theme->font);
   }
}
