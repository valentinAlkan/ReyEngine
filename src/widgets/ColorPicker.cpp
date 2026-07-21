#include "ColorPicker.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cctype>
#include <sstream>
#include <vector>

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
   _update_readout();
   if (publishEvent) {
      EventColorChanged event(this, getColor());
      publish(event);
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
void ColorPicker::_init() {
   if (!_readoutEdits[0]) {
      static constexpr const char* ROW_NAMES[NUM_READOUT_ROWS] = {"Hex", "Rgba", "Hsv"};
      static constexpr const char* ROW_LABELS[NUM_READOUT_ROWS] = {"HEX", "RGBA", "HSV"};
      for (int i = 0; i < NUM_READOUT_ROWS; i++) {
         _readoutLabels[i] = make_child<Label>(std::string("readoutLabel") + ROW_NAMES[i], ROW_LABELS[i]);
         _readoutEdits[i] = make_child<LineEdit>(std::string("readoutEdit") + ROW_NAMES[i]);
         _readoutEdits[i]->setDefaultText("");
         _readoutLabels[i]->setVisible(_readoutEnabled);
         _readoutEdits[i]->setVisible(_readoutEnabled);
         subscribe<LineEdit::EventTextEntered>(_readoutEdits[i], [this, i](const LineEdit::EventTextEntered&){_on_readout_entered(i);});
      }
   }
   _update_readout();
   _compute_appearance();
}

/////////////////////////////////////////////////////////////////////////////////////////
void ColorPicker::_compute_appearance() {
   const auto area = getSizeRect().embiggen(-PAD);
   if (theme && theme->font) _readoutLineHeight = theme->font->measure(" ").y;
   const float rowHeight = _readoutLineHeight + 8;
   constexpr float numRows = NUM_READOUT_ROWS;
   const float bottomHeight = _readoutEnabled ? max(SWATCH_HEIGHT, numRows * rowHeight + (numRows - 1) * ROW_GAP) : SWATCH_HEIGHT;
   const float stripCount = _alphaEnabled ? 2 : 1;
   const float contentHeight = max(0.0f, area.height - bottomHeight - PAD);
   _svField = {area.x, area.y, max(0.0f, area.width - stripCount * (STRIP_WIDTH + PAD)), contentHeight};
   _hueBar = {_svField.x + _svField.width + PAD, area.y, STRIP_WIDTH, contentHeight};
   _alphaBar = _alphaEnabled ? Rect<float>{_hueBar.x + STRIP_WIDTH + PAD, area.y, STRIP_WIDTH, contentHeight} : Rect<float>{0, 0, 0, 0};
   const float bottomTop = area.y + area.height - bottomHeight;
   if (_readoutEnabled) {
      _swatch = {area.x, bottomTop, SWATCH_PREVIEW_WIDTH, bottomHeight};
      _readout = {_swatch.x + _swatch.width + PAD, bottomTop, max(0.0f, area.width - SWATCH_PREVIEW_WIDTH - PAD), bottomHeight};
      //the label column and edit column are each aligned so the rows read as a table
      float labelWidth = 44;
      if (theme && theme->font) labelWidth = theme->font->measure("RGBA").x + 6;
      const float editX = _readout.x + labelWidth + PAD;
      const float editWidth = max(0.0f, _readout.x + _readout.width - editX);
      for (int i = 0; i < NUM_READOUT_ROWS; i++) {
         const float rowY = _readout.y + i * (rowHeight + ROW_GAP);
         if (_readoutLabels[i]) _readoutLabels[i]->setRect(_readout.x, rowY + (rowHeight - _readoutLineHeight) / 2, labelWidth, _readoutLineHeight);
         if (_readoutEdits[i]) _readoutEdits[i]->setRect(editX, rowY, editWidth, rowHeight);
      }
   } else {
      _swatch = {area.x, bottomTop, area.width, bottomHeight};
      _readout = {0, 0, 0, 0};
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
void ColorPicker::_update_readout() {
   if (!_readoutEdits[0]) return;
   const auto c = getColor();
   char buf[48];
   if (_alphaEnabled) snprintf(buf, sizeof(buf), "#%02X%02X%02X%02X", c.r, c.g, c.b, c.a);
   else               snprintf(buf, sizeof(buf), "#%02X%02X%02X", c.r, c.g, c.b);
   _readoutEdits[ROW_HEX]->setText(buf, true);
   _readoutLabels[ROW_RGBA]->setText(_alphaEnabled ? "RGBA" : "RGB");
   if (_alphaEnabled) snprintf(buf, sizeof(buf), "%d, %d, %d, %d", c.r, c.g, c.b, c.a);
   else               snprintf(buf, sizeof(buf), "%d, %d, %d", c.r, c.g, c.b);
   _readoutEdits[ROW_RGBA]->setText(buf, true);
   snprintf(buf, sizeof(buf), "%d, %d%%, %d%%", static_cast<int>(lround(_hue)), static_cast<int>(lround(_sat * 100)), static_cast<int>(lround(_val * 100)));
   _readoutEdits[ROW_HSV]->setText(buf, true);
}

/////////////////////////////////////////////////////////////////////////////////////////
void ColorPicker::_on_readout_entered(int row) {
   const string text = _readoutEdits[row]->getText();
   bool applied = false;
   switch (row) {
      case ROW_HEX: {
         string hex = text;
         hex.erase(remove_if(hex.begin(), hex.end(), [](unsigned char ch){return isspace(ch) || ch == '#';}), hex.end());
         const bool allHex = !hex.empty() && all_of(hex.begin(), hex.end(), [](unsigned char ch){return isxdigit(ch);});
         if (allHex && (hex.size() == 6 || hex.size() == 8)) {
            const unsigned long v = stoul(hex, nullptr, 16);
            const auto color = hex.size() == 8
               ? ColorRGBA(static_cast<int>(v >> 24 & 0xFF), static_cast<int>(v >> 16 & 0xFF), static_cast<int>(v >> 8 & 0xFF), static_cast<int>(v & 0xFF))
               : ColorRGBA(static_cast<int>(v >> 16 & 0xFF), static_cast<int>(v >> 8 & 0xFF), static_cast<int>(v & 0xFF), static_cast<int>(lround(_alpha * 255)));
            toHSV(color, _hue, _sat, _val, _alpha);
            applied = true;
         }
      } break;
      case ROW_RGBA: {
         string t = text;
         for (auto& ch : t) if (ch == ',') ch = ' ';
         istringstream ss(t);
         vector<int> vals;
         int v;
         while (ss >> v) vals.push_back(v);
         if (vals.size() == 3 || vals.size() == 4) {
            auto toByte = [](int val){return clamp(val, 0, 255);};
            const int a = vals.size() == 4 ? toByte(vals[3]) : static_cast<int>(lround(_alpha * 255));
            toHSV(ColorRGBA(toByte(vals[0]), toByte(vals[1]), toByte(vals[2]), a), _hue, _sat, _val, _alpha);
            applied = true;
         }
      } break;
      case ROW_HSV: {
         string t = text;
         for (auto& ch : t) if (ch == ',' || ch == '%') ch = ' ';
         istringstream ss(t);
         float h, s, v;
         if (ss >> h >> s >> v) {
            _hue = clamp(h, 0.0f, 360.0f);
            if (_hue == 360.0f) _hue = 0;
            _sat = clamp(s / 100.0f, 0.0f, 1.0f);
            _val = clamp(v / 100.0f, 0.0f, 1.0f);
            applied = true;
         }
      } break;
   }
   if (applied) {
      EventColorChanged changed(this, getColor());
      publish(changed);
      EventColorPicked picked(this, getColor());
      publish(picked);
   }
   _update_readout(); //canonicalize the entered value, or restore the current one if it didn't parse
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
   _update_readout();
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
   //tile the segments on integer boundaries - the draw call truncates floats, and fractional
   // segment heights would otherwise leave single-pixel seams between segments
   const float segmentHeight = _hueBar.height / 6.0f;
   for (int i = 0; i < 6; i++) {
      const float y0 = floorf(_hueBar.y + i * segmentHeight);
      const float y1 = floorf(i == 5 ? _hueBar.y + _hueBar.height : _hueBar.y + (i + 1) * segmentHeight);
      drawRectangleGradientV({_hueBar.x, y0, _hueBar.width, y1 - y0}, HUE_STOPS[i], HUE_STOPS[i + 1]);
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
}
