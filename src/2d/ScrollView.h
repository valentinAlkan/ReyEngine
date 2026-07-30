#pragma once
#include <algorithm>
#include <memory>
#include "Slider.h"

namespace ReyEngine {
   // Reusable scroll *model* for a virtualized widget. The host owns rendering and
   // input; ScrollView owns only the scroll offset: it clamps the offset to the
   // content, keeps an attached Slider's range/page-size/grabber in sync, and answers
   // "scroll so this span is visible" for things like caret-follow.
   //
   // It is deliberately NOT a Widget and creates no children, so any widget can add one
   // as a member without inheritance/access entanglement. The host creates the Slider
   // (it has access to its own protected make_child), hands it over with attachVBar() /
   // attachHBar(), subscribes the bar's value-changed to setOffsetY(value, /*fromBar*/true),
   // and positions the bar's rect itself.
   //
   // Both axes are supported. They are independent except in layout(): showing one bar
   // eats space along the other axis, which can force the other bar to appear - see the
   // three-argument layout() overload.
   class ScrollView {
   public:
      // One scroll axis. Both of ScrollView's axes are instances of this; the axis knows
      // nothing about orientation, since a Slider's value API is direction-agnostic
      // (SliderType only affects how it draws and reads input).
      class Axis {
      public:
         //hand over the host-owned scrollbar; Axis only drives its public API
         void attachBar(std::shared_ptr<Slider> bar){ _bar = std::move(bar); }

         [[nodiscard]] float offset() const { return _offset; }
         [[nodiscard]] float maxOffset() const { return _maxOffset; }
         [[nodiscard]] float viewport() const { return _viewport; }
         [[nodiscard]] float content() const { return _content; }
         [[nodiscard]] bool  needsBar() const { return _maxOffset > 0.0f; } //content longer than viewport

         // Recompute the scroll limit from the latest sizes, clamp the current offset, and
         // refresh the bar. Call whenever content size, viewport size, or the bar changes.
         void layout(float contentLen, float viewportLen) {
            _content  = contentLen;
            _viewport = viewportLen;
            _maxOffset = std::max(0.0f, _content - _viewport);
            _offset = std::clamp(_offset, 0.0f, _maxOffset);
            syncBar();
         }

         // Set the offset (clamped). `fromBar` = the change came from the user dragging the
         // bar, so we must not write back to it (which would fight the drag / loop).
         void setOffset(float v, bool fromBar = false) {
            _offset = std::clamp(v, 0.0f, _maxOffset);
            if (!fromBar) syncBar();
         }

         void scrollBy(float delta){ setOffset(_offset + delta); }

         // Shift the offset so the content-space span [spanStart, spanEnd] sits inside the
         // viewport. No-op when it already does. Used for caret-follow.
         // (Not named near/far - those are macros in some Windows headers.)
         void ensureVisible(float spanStart, float spanEnd) {
            if (spanStart < _offset)                  setOffset(spanStart);
            else if (spanEnd > _offset + _viewport)   setOffset(spanEnd - _viewport);
         }

      private:
         void syncBar() {
            if (!_bar) return;
            _bar->setVisible(needsBar());
            if (!needsBar()) return;
            _bar->setRange({0.0, static_cast<double>(_maxOffset)});
            _bar->setVisibleAmount(static_cast<double>(_viewport)); //page size -> grabber proportion = viewport/content
            _bar->setSliderValue(_offset, /*publish*/false);        //reflect offset without re-emitting
         }

         std::shared_ptr<Slider> _bar;
         float _offset    = 0; //current scroll along this axis, in pixels of content space
         float _content   = 0; //total content length
         float _viewport  = 0; //visible length
         float _maxOffset = 0; //= max(0, content - viewport)
      };

      //direct access, for callers that want to treat the axes uniformly
      [[nodiscard]] Axis& axisX(){ return _x; }
      [[nodiscard]] Axis& axisY(){ return _y; }
      [[nodiscard]] const Axis& axisX() const { return _x; }
      [[nodiscard]] const Axis& axisY() const { return _y; }

      // ---- vertical ----------------------------------------------------------------
      void attachVBar(std::shared_ptr<Slider> bar){ _y.attachBar(std::move(bar)); }
      [[nodiscard]] float offsetY() const { return _y.offset(); }
      [[nodiscard]] float maxOffsetY() const { return _y.maxOffset(); }
      [[nodiscard]] bool  needsVBar() const { return _y.needsBar(); }
      void setOffsetY(float y, bool fromBar = false){ _y.setOffset(y, fromBar); }
      void scrollByY(float dy){ _y.scrollBy(dy); }
      void ensureVisibleY(float top, float bottom){ _y.ensureVisible(top, bottom); }

      // ---- horizontal --------------------------------------------------------------
      void attachHBar(std::shared_ptr<Slider> bar){ _x.attachBar(std::move(bar)); }
      [[nodiscard]] float offsetX() const { return _x.offset(); }
      [[nodiscard]] float maxOffsetX() const { return _x.maxOffset(); }
      [[nodiscard]] bool  needsHBar() const { return _x.needsBar(); }
      void setOffsetX(float x, bool fromBar = false){ _x.setOffset(x, fromBar); }
      void scrollByX(float dx){ _x.scrollBy(dx); }
      void ensureVisibleX(float left, float right){ _x.ensureVisible(left, right); }

      //combined offset, handy for translating a whole point between content and viewport space
      [[nodiscard]] Pos<float> offset() const { return {_x.offset(), _y.offset()}; }
      void scrollBy(const Vec2<float>& delta){ _x.scrollBy(delta.x); _y.scrollBy(delta.y); }

      // ---- layout ------------------------------------------------------------------

      // Vertical only. Kept for hosts that manage their own horizontal gutter.
      void layout(float contentHeight, float viewportHeight) {
         _y.layout(contentHeight, viewportHeight);
      }

      // Both axes, with the viewport already decided by the caller.
      void layout(const Size<float>& content, const Size<float>& viewport) {
         _x.layout(content.x, viewport.x);
         _y.layout(content.y, viewport.y);
      }

      // Both axes, resolving the gutters for us. `viewport` is the space available with no
      // bars showing; each bar that ends up visible consumes `barThickness` from the other
      // axis. That coupling is why this can't be done one axis at a time: a horizontal bar
      // shortens the viewport height, which can be what pushes the vertical bar into
      // existence (and vice versa). One correction pass settles it - a second can't change
      // the answer, since a bar already counted can only stay needed.
      void layout(const Size<float>& content, const Size<float>& viewport, float barThickness) {
         bool needH = content.x > viewport.x;
         bool needV = content.y > viewport.y;
         if (needH && !needV) needV = content.y > viewport.y - barThickness;
         if (needV && !needH) needH = content.x > viewport.x - barThickness;

         _x.layout(content.x, viewport.x - (needV ? barThickness : 0.0f));
         _y.layout(content.y, viewport.y - (needH ? barThickness : 0.0f));
      }

   private:
      Axis _x;
      Axis _y;
   };
}
