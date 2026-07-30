// Unit test for the ScrollView scroll model (step 3 of the ScrollArea scissor refactor).
//
// Pure model, no rendering and no Slider attached, so this needs no GL context.
// Covers both axes, clamping, ensureVisible, and the gutter-coupling in the
// three-argument layout() - where showing one bar can force the other one to appear.
#include "ScrollView.h"
#include <iostream>
#include <string>

using namespace ReyEngine;
using namespace std;

namespace {
int failures = 0;

void check(bool ok, const string& what){
   cout << (ok ? "PASS  " : "FAIL  ") << what << "\n";
   if (!ok) failures++;
}
void checkEq(float got, float want, const string& what){
   bool ok = std::abs(got - want) < 0.001f;
   cout << (ok ? "PASS  " : "FAIL  ") << what << "  (got " << got << ", want " << want << ")\n";
   if (!ok) failures++;
}
} // namespace

int main(){
   // ---------------------------------------------------------------- single axis basics
   {
      cout << "\n-- axis: limits and clamping --\n";
      ScrollView::Axis a;
      a.layout(/*content*/1000, /*viewport*/400);
      checkEq(a.maxOffset(), 600, "maxOffset = content - viewport");
      check(a.needsBar(), "needs a bar when content exceeds viewport");

      a.setOffset(100);
      checkEq(a.offset(), 100, "offset set within range");
      a.setOffset(99999);
      checkEq(a.offset(), 600, "offset clamped to maxOffset");
      a.setOffset(-50);
      checkEq(a.offset(), 0, "offset clamped to zero");

      a.setOffset(300);
      a.scrollBy(50);
      checkEq(a.offset(), 350, "scrollBy adds");
      a.scrollBy(-1000);
      checkEq(a.offset(), 0, "scrollBy clamps");
   }

   {
      cout << "\n-- axis: content smaller than viewport --\n";
      ScrollView::Axis a;
      a.layout(/*content*/100, /*viewport*/400);
      checkEq(a.maxOffset(), 0, "no scroll range when content fits");
      check(!a.needsBar(), "no bar needed when content fits");
      a.setOffset(50);
      checkEq(a.offset(), 0, "offset pinned to zero when content fits");
   }

   {
      cout << "\n-- axis: shrinking content re-clamps a live offset --\n";
      ScrollView::Axis a;
      a.layout(1000, 400);
      a.setOffset(600);
      a.layout(500, 400);              //content shrank; 600 is now out of range
      checkEq(a.maxOffset(), 100, "limit follows new content");
      checkEq(a.offset(), 100, "existing offset re-clamped by layout");
   }

   {
      cout << "\n-- axis: ensureVisible --\n";
      ScrollView::Axis a;
      a.layout(/*content*/1000, /*viewport*/100);

      a.setOffset(0);
      a.ensureVisible(20, 40);
      checkEq(a.offset(), 0, "already-visible span does not move the offset");

      a.ensureVisible(300, 320);       //below the viewport
      checkEq(a.offset(), 220, "scrolls down just far enough to reveal the end");

      a.ensureVisible(50, 70);         //above the viewport
      checkEq(a.offset(), 50, "scrolls up to the start of the span");
   }

   // ------------------------------------------------------- two axes, explicit viewport
   {
      cout << "\n-- view: both axes are independent --\n";
      ScrollView v;
      v.layout(Size<float>(1000, 500), Size<float>(400, 400));
      checkEq(v.maxOffsetX(), 600, "x limit");
      checkEq(v.maxOffsetY(), 100, "y limit");
      check(v.needsHBar(), "h bar needed");
      check(v.needsVBar(), "v bar needed");

      v.setOffsetX(123);
      v.setOffsetY(45);
      checkEq(v.offsetX(), 123, "x offset independent");
      checkEq(v.offsetY(), 45,  "y offset independent");
      checkEq(v.offset().x, 123, "combined offset x");
      checkEq(v.offset().y, 45,  "combined offset y");
   }

   {
      cout << "\n-- view: Y-only layout leaves X alone (back-compat path) --\n";
      ScrollView v;
      v.layout(/*contentHeight*/1000, /*viewportHeight*/400);
      checkEq(v.maxOffsetY(), 600, "y limit set by the 2-arg overload");
      checkEq(v.maxOffsetX(), 0, "x untouched");
      check(!v.needsHBar(), "no h bar from a y-only layout");
   }

   // --------------------------------------------------- gutter coupling (the subtle bit)
   {
      cout << "\n-- view: neither axis overflows, no gutters consumed --\n";
      ScrollView v;
      v.layout(Size<float>(100, 100), Size<float>(400, 400), /*barThickness*/20);
      check(!v.needsHBar() && !v.needsVBar(), "no bars when everything fits");
      checkEq(v.axisX().viewport(), 400, "x viewport keeps full width");
      checkEq(v.axisY().viewport(), 400, "y viewport keeps full height");
   }

   {
      cout << "\n-- view: one axis overflows, its bar eats the other's viewport --\n";
      ScrollView v;
      // content is wider than the viewport but much shorter -> only an h bar is needed,
      // and it consumes 20px of height.
      v.layout(Size<float>(1000, 50), Size<float>(400, 400), /*barThickness*/20);
      check(v.needsHBar(),  "h bar needed (content wider)");
      check(!v.needsVBar(), "v bar still not needed (content much shorter)");
      checkEq(v.axisY().viewport(), 380, "h bar consumed height from the y viewport");
      checkEq(v.axisX().viewport(), 400, "x viewport keeps full width (no v bar)");
   }

   {
      cout << "\n-- view: showing one bar FORCES the other (the coupling case) --\n";
      ScrollView v;
      // Width overflows outright. Height fits in 400 but NOT in 400-20, so the vertical
      // bar only becomes necessary because the horizontal bar took 20px.
      v.layout(Size<float>(1000, 390), Size<float>(400, 400), /*barThickness*/20);
      check(v.needsHBar(), "h bar needed (content wider)");
      check(v.needsVBar(), "v bar forced into existence by the h bar's gutter");
      checkEq(v.axisY().viewport(), 380, "y viewport reduced by h bar");
      checkEq(v.axisX().viewport(), 380, "x viewport reduced by the now-needed v bar");
      checkEq(v.maxOffsetY(), 10, "y scroll range is exactly the overflow");
   }

   {
      cout << "\n-- view: the same coupling in the opposite direction --\n";
      ScrollView v;
      v.layout(Size<float>(390, 1000), Size<float>(400, 400), /*barThickness*/20);
      check(v.needsVBar(), "v bar needed (content taller)");
      check(v.needsHBar(), "h bar forced into existence by the v bar's gutter");
      checkEq(v.axisX().viewport(), 380, "x viewport reduced by v bar");
      checkEq(v.maxOffsetX(), 10, "x scroll range is exactly the overflow");
   }

   {
      cout << "\n-- view: near-miss does NOT spuriously add a bar --\n";
      ScrollView v;
      // Height fits even after the h bar's gutter, so no v bar should appear.
      v.layout(Size<float>(1000, 300), Size<float>(400, 400), /*barThickness*/20);
      check(v.needsHBar(),  "h bar needed");
      check(!v.needsVBar(), "v bar NOT added when content still fits after the gutter");
      checkEq(v.axisX().viewport(), 400, "x viewport not reduced");
   }

   cout << "\n" << (failures ? "FAILURES: " + to_string(failures) : "all scrollview tests passed") << "\n";
   return failures ? 1 : 0;
}
