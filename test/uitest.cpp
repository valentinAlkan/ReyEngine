#include "Application.h"
#include "Window.h"
#include "Layout.h"
#include "Button.h"
#include "LineEdit.h"

using namespace std;
using namespace ReyEngine;

class ThemeExplorer : public Widget {
   std::array<std::tuple<Rect<float>, std::string, ColorRGBA>, 20> _rects;
   void _on_rect_changed() override {
      if (!_has_inited) return;
      constexpr float RECT_GAP = 2.0;
      Rect<float> lastRect = {};
      for (auto& [rect, text, color] : _rects){
         rect.setPos(lastRect.bottomLeft() + Pos<float>(0, RECT_GAP));
         rect.setWidth(getWidth());
         rect.setHeight(30);
         lastRect = rect;
      }

      auto setPairs = [&](int index, auto text, auto color){
         get<1>(_rects.at(index)) = text;
         get<2>(_rects.at(index)) = color;
      };
      setPairs(0, "Background: colorPrimary", theme->background.colorPrimary);
      setPairs(1, "Background: colorSecondary", theme->background.colorSecondary);
      setPairs(2, "Background: colorTertiary", theme->background.colorTertiary);
      setPairs(3, "Background: colorDisabled", theme->background.colorHighlight);
      setPairs(4, "Background: colorActive", theme->background.colorActive);
      setPairs(5, "Background: colorDisabled", theme->background.colorDisabled);
   }
   void render2D() const override {
      for (const auto& [rect, text, color] : _rects){
         drawRectangle(rect, color);
         drawText(text, rect.topLeft(), theme->font, color.getReadableTextColor(), theme->font->size, theme->font->spacing);
      }
      drawRectangleLines(getSizeRect(), 1.0, Colors::white);
   }
};

int main() {
//   ArgParse args;
//   args.defineArg(RuntimeArg("--fontPath", "fontPath", 0, RuntimeArg::ArgType::FLAG));
//   args.parseArgs(argc, argv);

   constexpr float ROW_HEIGHT = 30;

   auto& window = Application::createWindowPrototype("UITest", 1920, 1080, {WindowFlags::RESIZE}, 60)->createWindow();
   auto root = window.getCanvas();

   auto mainHLayout = make_child<Layout>(root, "mainVLayout", Layout::LayoutDir::HORIZONTAL);
   mainHLayout->getTheme().layoutMargin = 2;
   mainHLayout->getTheme().widgetPadding = {2,2,2,2};
   mainHLayout->setAnchoring(ReyEngine::Anchor::FILL);
   auto vlayoutl = make_child<Layout>(mainHLayout, "vlayoutl", Layout::LayoutDir::VERTICAL);
   auto vlayoutm1 = make_child<Layout>(mainHLayout, "vlayoutm1", Layout::LayoutDir::VERTICAL);
   auto vlayoutm2 = make_child<Layout>(mainHLayout, "vlayoutm2", Layout::LayoutDir::VERTICAL);
   auto vlayoutr = make_child<Layout>(mainHLayout, "vlayoutr", Layout::LayoutDir::VERTICAL);
   //add each widget type
   auto pushbutton = make_child<PushButton>(vlayoutl, "pushbutton");
   pushbutton->setMaxHeight(ROW_HEIGHT);
   auto lineedit = make_child<LineEdit>(vlayoutl, "lineedit", "LineEdit");
   auto checkbox = make_child<CheckBox>(vlayoutl, "checkbox", "This is a checkbox");
   checkbox->setMaxHeight(ROW_HEIGHT);

   make_child<ThemeExplorer>(vlayoutr, "themeExplorer");


   window.exec();

}