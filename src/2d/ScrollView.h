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
   // (it has access to its own protected make_child), hands it over with attachVBar(),
   // subscribes the bar's value-changed to setOffsetY(value, /*fromBar*/true), and
   // positions the bar's rect itself.
   //
   // Only the vertical axis is implemented for now. The API is intentionally Y-suffixed
   // so an analogous X axis can be added later without disturbing existing Y call sites.
   class ScrollView {
   public:
      //hand over the host-owned vertical scrollbar; ScrollView only drives its public API
      void attachVBar(std::shared_ptr<Slider> bar){ _vbar = std::move(bar); }

      [[nodiscard]] float offsetY() const { return _offsetY; }
      [[nodiscard]] float maxOffsetY() const { return _maxOffsetY; }
      [[nodiscard]] bool  needsVBar() const { return _maxOffsetY > 0.0f; } //content taller than viewport

      // Recompute the scroll limit from the latest sizes, clamp the current offset, and
      // refresh the bar. Call whenever content height, viewport height, or the bar changes.
      void layout(float contentHeight, float viewportHeight) {
         _contentH  = contentHeight;
         _viewportH = viewportHeight;
         _maxOffsetY = std::max(0.0f, _contentH - _viewportH);
         _offsetY = std::clamp(_offsetY, 0.0f, _maxOffsetY);
         syncBar();
      }

      // Set the offset (clamped). `fromBar` = the change came from the user dragging the
      // bar, so we must not write back to it (which would fight the drag / loop).
      void setOffsetY(float y, bool fromBar = false) {
         _offsetY = std::clamp(y, 0.0f, _maxOffsetY);
         if (!fromBar) syncBar();
      }

      void scrollByY(float dy){ setOffsetY(_offsetY + dy); }

      // Shift the offset so the content-space span [top, bottom] sits inside the viewport.
      // No-op when it already does. Used for caret-follow.
      void ensureVisibleY(float top, float bottom) {
         if (top < _offsetY)                    setOffsetY(top);
         else if (bottom > _offsetY + _viewportH) setOffsetY(bottom - _viewportH);
      }

   private:
      void syncBar() {
         if (!_vbar) return;
         _vbar->setVisible(needsVBar());
         if (!needsVBar()) return;
         _vbar->setRange({0.0, static_cast<double>(_maxOffsetY)});
         _vbar->setVisibleAmount(static_cast<double>(_viewportH)); //page size -> grabber proportion = viewport/content
         _vbar->setSliderValue(_offsetY, /*publish*/false);        //reflect offset without re-emitting
      }

      std::shared_ptr<Slider> _vbar;
      float _offsetY    = 0; //current vertical scroll, in pixels of content space
      float _contentH   = 0; //total content height
      float _viewportH  = 0; //visible height
      float _maxOffsetY = 0; //= max(0, content - viewport)
   };
}
