#include "Editor.h"
#include "Panel.hpp"
#include "Button.hpp"
#include "Workspace.h"

using namespace GFCSDraw;
using namespace std;

/////////////////////////////////////////////////////////////////////////////////////////
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
   mainHLayout->getTheme()->background.colorPrimary.set(GFCSDraw::Colors::lightGray);
   mainPanel->addToLayout(mainHLayout);

   //create left panel
   auto mainHLayoutLeftPanel = make_shared<VLayout>("mainHLayoutLeftPanel", Rect<int>());
   mainHLayout->addChild(mainHLayoutLeftPanel);

   //create scene tree panel
   {
      auto treeRoot = make_shared<TreeItem>("SceneRoot");
      sceneTree = make_shared<SceneTree>("sceneTree", Rect<int>());
      sceneTree->setRoot(treeRoot);
      mainHLayoutLeftPanel->addChild(sceneTree);
      sceneTree->getTheme()->background.colorPrimary.set(GFCSDraw::Colors::yellow);
   }

   //create widget tree panel
   {
      widgetTree = make_shared<WidgetTree>("widgetTree", Rect<int>());
      mainHLayoutLeftPanel->addChild(widgetTree);
      widgetTree->getTheme()->background.colorPrimary.set(GFCSDraw::Colors::green);
   }


   //create the (blank) workspace
   auto workspace = make_shared<Workspace>("Workspace", Rect<int>());
   workspace->getTheme()->background.colorPrimary.set(COLORS::gray);
   mainHLayout->addChild(workspace);

   //create the right panel inspector
   inspector = make_shared<Inspector>("Inspector", Rect<int>());
   mainHLayout->addChild(inspector);
   //connect events
   auto onWidgetAdded = [&](const Workspace::EventWidgetAdded& event){
      Application::printDebug() << "Inspecting widget " << event.widget->getName() << endl;
      inspector->inspect(event.widget);
   };
   inspector->subscribe<Workspace::EventWidgetAdded>(workspace, onWidgetAdded);

   //set the panel ratios
   mainHLayout->childScales.set(0, 0.15);
   mainHLayout->childScales.set(1, 0.70);
   mainHLayout->childScales.set(2, 0.15);
}

///////////////////////////////////////////////////////////////////////////////////////////
//void Editor::inspect(std::shared_ptr<BaseWidget> widget){
//   if (inspector) {
//      inspector->inspect(widget);
//   }
//}