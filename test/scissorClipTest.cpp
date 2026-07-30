// Integration test for step 1 + step 2 of the ScrollArea scissor refactor.
//
//  * getChildClipRect() actually clips a widget's child subtree
//  * nested clips intersect (a child cannot widen its parent's region)
//  * a nested clip's scope exit RESTORES the parent region instead of disabling
//    scissoring outright  <-- the F1 bug; this is the assertion that would fail before
//
// Renders into an offscreen canvas and reads pixels back, so no visible window is needed
// beyond the GL context.
#include "Window.h"
#include <iostream>
#include <vector>

using namespace ReyEngine;
using namespace std;

namespace {

int failures = 0;
void check(bool ok, const string& what){
   cout << (ok ? "PASS  " : "FAIL  ") << what << "\n";
   if (!ok) failures++;
}

//fills its entire rect with a flat color
class Filler : public Widget {
public:
   REYENGINE_OBJECT(Filler)
   explicit Filler(const ColorRGBA& c) : color(c) {}
   void render2D(RenderContext&) const override { drawRectangle(getSizeRect(), color); }
   ColorRGBA color;
};

//clips its children (and their descendants) to a local-space region
class Clipper : public Widget {
public:
   REYENGINE_OBJECT(Clipper)
   explicit Clipper(const Rect<float>& clip) : clip(clip) {}
   void render2D(RenderContext&) const override {}
   std::optional<Rect<R_FLOAT>> getChildClipRect() const override { return clip; }
   Rect<float> clip;
};

//canvas we can drive a single frame on, and read back
class ProbeCanvas : public Canvas {
public:
   REYENGINE_OBJECT(ProbeCanvas)
   void renderOneFrame(){
      RenderContext ctx(_renderTarget);
      renderProcess(ctx);
   }
   const RenderTarget& target() const { return readRenderTarget(); }
};

//snapshot of a canvas' render target, in natural top-left-origin coordinates
struct Snapshot {
   explicit Snapshot(const ProbeCanvas& c){
      img = LoadImageFromTexture(c.target().getTexture());
      ImageFlipVertical(&img);   //render textures come back bottom-up
   }
   ~Snapshot(){ UnloadImage(img); }
   Color at(int x, int y) const { return GetImageColor(img, x, y); }
   bool isColor(int x, int y, const ColorRGBA& want, int tol = 6) const {
      auto got = at(x, y);
      Color w = want;
      return abs(got.r - w.r) <= tol && abs(got.g - w.g) <= tol && abs(got.b - w.b) <= tol;
   }
   Image img;
};

} // namespace

int main(){
   SetTraceLogLevel(LOG_WARNING);
   auto& window = Application::createWindowPrototype("cliptest", 500, 500, {}, 0)->createWindow();
   auto root = window.getCanvas();
   root->removeAllChildren();

   constexpr float SZ = 400;
   const auto kRed   = Colors::red;
   const auto kGreen = Colors::green;

   // ---------------------------------------------------------------- test 1: basic clip
   {
      auto canvas = make_child<ProbeCanvas>(root, "canvas1");
      canvas->setRect({0, 0, SZ, SZ});
      auto clipper = make_child<Clipper>(canvas, "clipper", Rect<float>(50, 50, 100, 100));
      clipper->setRect({0, 0, SZ, SZ});
      auto filler = make_child<Filler>(clipper, "filler", kRed);
      filler->setRect({0, 0, SZ, SZ});

      canvas->renderOneFrame();
      Snapshot s(*canvas);

      cout << "\n-- test 1: getChildClipRect clips the child subtree --\n";
      check( s.isColor(100, 100, kRed), "inside clip region is filled");
      check(!s.isColor( 10,  10, kRed), "outside clip (top-left) is NOT filled");
      check(!s.isColor(200, 200, kRed), "outside clip (bottom-right) is NOT filled");
      check(!s.isColor( 45, 100, kRed), "just left of clip edge is NOT filled");
      check( s.isColor( 55, 100, kRed), "just inside clip edge is filled");
   }

   // ------------------------------------------- test 2: nesting + parent restore (F1)
   {
      auto canvas = make_child<ProbeCanvas>(root, "canvas2");
      canvas->setRect({0, 0, SZ, SZ});

      // outer clip: a small window
      auto outer = make_child<Clipper>(canvas, "outer", Rect<float>(50, 50, 100, 100));
      outer->setRect({0, 0, SZ, SZ});

      // inner clip asks for a region LARGER than outer -> must not widen it
      auto inner = make_child<Clipper>(outer, "inner", Rect<float>(0, 0, SZ, SZ));
      inner->setRect({0, 0, SZ, SZ});
      auto greenFill = make_child<Filler>(inner, "green", kGreen);
      greenFill->setRect({0, 0, SZ, SZ});

      // sibling drawn AFTER the inner clipper's scope has ended, positioned entirely
      // outside `outer`. If ending the inner scope disabled scissoring (the old bug)
      // this becomes visible.
      auto redFill = make_child<Filler>(outer, "red", kRed);
      redFill->setRect({200, 200, 100, 100});

      canvas->renderOneFrame();
      Snapshot s(*canvas);

      cout << "\n-- test 2: nested clips intersect, and restore on scope exit --\n";
      check( s.isColor(100, 100, kGreen), "inner content visible inside intersection");
      check(!s.isColor( 10,  10, kGreen), "inner clip did NOT widen the outer region");
      check(!s.isColor(250, 250, kRed),   "sibling after nested scope is still clipped (F1)");
      check(!s.isColor(250, 250, kGreen), "no stray inner content outside outer region");
   }

   // ------------------------------------------------- test 3: no hook == no clipping
   {
      auto canvas = make_child<ProbeCanvas>(root, "canvas3");
      canvas->setRect({0, 0, SZ, SZ});
      auto plain = make_child<Widget>(canvas, "plain");
      plain->setRect({0, 0, SZ, SZ});
      auto filler = make_child<Filler>(plain, "filler", kRed);
      filler->setRect({0, 0, SZ, SZ});

      canvas->renderOneFrame();
      Snapshot s(*canvas);

      cout << "\n-- test 3: control, widget without the hook is unclipped --\n";
      check(s.isColor( 10,  10, kRed), "top-left filled");
      check(s.isColor(200, 200, kRed), "bottom-right filled");
   }

   cout << "\n" << (failures ? "FAILURES: " + to_string(failures) : "all clip tests passed") << "\n";
   return failures ? 1 : 0;
}
