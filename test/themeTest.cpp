#include "Application.h"
#include "Window.h"
#include "Layout.h"
#include "Button.h"
#include "LineEdit.h"
#include "MenuBar.h"

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
      setPairs(3, "Background: colorHighlight", theme->background.colorHighlight);
      setPairs(4, "Background: colorActive1", theme->background.colorActive1);
      setPairs(5, "Background: colorActive2", theme->background.colorActive2);
      setPairs(6, "Background: colorDisabled", theme->background.colorDisabled);

      setPairs(7, "Foreground: colorPrimary", theme->foreground.colorPrimary);
      setPairs(8, "Foreground: colorSecondary", theme->foreground.colorSecondary);
      setPairs(9, "Foreground: colorTertiary", theme->foreground.colorTertiary);
      setPairs(10, "Foreground: colorHighlight", theme->foreground.colorHighlight);
      setPairs(11, "Foreground: colorActive1", theme->foreground.colorActive1);
      setPairs(12, "Foreground: colorActive2", theme->foreground.colorActive2);
      setPairs(13, "Foreground: colorDisabled", theme->foreground.colorDisabled);
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
   auto toggleButton = make_child<ToggleButton>(vlayoutl, "toggleButton");
   auto lineEditHLayout = make_child<Layout>(vlayoutl, "lineEditHLayout", Layout::LayoutDir::HORIZONTAL);
   auto lineEdit = make_child<LineEdit>(lineEditHLayout, "lineedit", "LineEdit");
   auto checkbox = make_child<CheckBox>(lineEditHLayout, "checkbox", "Enabled");
   checkbox->setChecked(true);

   auto menuBar = make_child<MenuBar>(vlayoutl, "menubar");
   auto fileMenu = menuBar->createDropDown("File");
   fileMenu->push_back("this", "is", "some", "items");

   make_child<ThemeExplorer>(vlayoutr, "themeExplorer");


   for (auto& child : vlayoutl->getChildren()){
      child->as<Widget>().value()->setMaxHeight(ROW_HEIGHT);
   }

   auto cbLineEditDisabled = [lineEdit](const CheckBox::ButtonToggleEvent& e){
      lineEdit->setEnabled(e.button().getDown());
   };
   lineEdit->subscribe<CheckBox::ButtonToggleEvent>(checkbox, cbLineEditDisabled);

   window.exec();

}