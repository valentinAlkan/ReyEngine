#include "Editor.h"
#include "Panel.hpp"
#include "Button.hpp"

using namespace GFCSDraw;
using namespace std;

void Editor::_init() {
   auto mainPanel = make_shared<Panel>("MainPanel", Rect<int>());
   mainPanel->setLayout<VLayout>();
   addChild(mainPanel);

   auto menuBarPanel= make_shared<Panel>("menuBarPanel", Rect<int>());
   menuBarPanel->setLayout<HLayout>();
   mainPanel->addToLayout(menuBarPanel);
   //set blue background (you gotta color it hard...so they can *see* it)
   menuBarPanel->getTheme()->background.colorPrimary.set(GFCSDraw::Colors::blue);
   //set menubar size
   menuBarPanel->setMaxSize({0,25});
   //add some buttons to the menu bar
   auto fileButton  = std::make_shared<PushButton>("fileBtn", Rect<int>());
   fileButton->setMaxSize({100,99999});
   menuBarPanel->addToLayout(fileButton);

   //create the workspace
   auto mainHLayout = make_shared<HLayout>("mainHLayout", Rect<int>());
   mainHLayout->getTheme()->background.colorPrimary.set(GFCSDraw::Colors::yellow);
   mainPanel->addToLayout(mainHLayout);

   //create tree panel
   auto treePanel = make_shared<Panel>("treePanel", Rect<int>());
   mainHLayout->addChild(treePanel);
   treePanel->getTheme()->background.colorPrimary.set(GFCSDraw::Colors::green);
//
//      //create the (blank) workspace
//      // todo: this should be a workspace widget
   auto workspace = make_shared<Control>("Panel", Rect<int>());
   workspace->getTheme()->background.colorPrimary.set(COLORS::gray);
   mainHLayout->addChild(workspace);
//
//      //create the rightpanel
   auto rightPanel = make_shared<Panel>("rightPanel", Rect<int>());
   rightPanel->getTheme()->background.colorPrimary.set(GFCSDraw::Colors::red);
   mainHLayout->addChild(rightPanel);

   //set the panel ratios
   mainHLayout->childScales.set(0, 0.15);
   mainHLayout->childScales.set(1, 0.70);
   mainHLayout->childScales.set(2, 0.15);
}