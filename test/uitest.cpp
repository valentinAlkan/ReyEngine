#include "Application.h"
#include "Window.h"
#include "Layout.h"
#include "Button.h"

using namespace std;
using namespace ReyEngine;

int main() {
//   ArgParse args;
//   args.defineArg(RuntimeArg("--fontPath", "fontPath", 0, RuntimeArg::ArgType::FLAG));
//   args.parseArgs(argc, argv);

   auto& window = Application::createWindowPrototype("UITest", 1920, 1080, {WindowFlags::RESIZE}, 60)->createWindow();
   auto root = window.getCanvas();

   auto mainHLayout = make_child<Layout>(root, "mainVLayout", Layout::LayoutDir::HORIZONTAL);
   mainHLayout->setAnchoring(ReyEngine::Anchor::FILL);
   auto vlayoutl = make_child<Layout>(mainHLayout, "vlayoutl", Layout::LayoutDir::VERTICAL);
   auto vlayoutm = make_child<Layout>(mainHLayout, "vlayoutm", Layout::LayoutDir::VERTICAL);
   auto vlayoutr = make_child<Layout>(mainHLayout, "vlayoutr", Layout::LayoutDir::VERTICAL);
   //add each widget type
   auto pushbutton = make_child<PushButton>(vlayoutl, "pushbutton");
   pushbutton->setMaxHeight(30);

   window.exec();

}