#include "Editor.h"
#include "Panel.hpp"
#include "Button.h"
#include "Workspace.h"

using namespace ReyEngine;
using namespace std;

/////////////////////////////////////////////////////////////////////////////////////////
void Editor::_init() {
   auto mainPanel = make_shared<Panel>("MainPanel");
   mainPanel->setLayout<VLayout>();
   addChild(mainPanel);

   auto menuBarPanel= make_shared<Panel>("menuBarPanel");
   menuBarPanel->setLayout<HLayout>();
   mainPanel->addToLayout(menuBarPanel);
   //set blue background (you gotta color it hard...so they can *see* it)
   menuBarPanel->getTheme()->background.colorPrimary.set(ReyEngine::Colors::blue);
   //set menubar size
   menuBarPanel->setMaxSize({0,25});
   //add some buttons to the menu bar
   auto fileButton  = std::make_shared<PushButton>("fileBtn");
   fileButton->setMaxSize({100,99999});
   menuBarPanel->addToLayout(fileButton);

   //create the workspace
   auto mainHLayout = make_shared<HLayout>("mainHLayout");
   mainHLayout->getTheme()->background.colorPrimary.set(ReyEngine::Colors::lightGray);
   mainPanel->addToLayout(mainHLayout);

   //create left panel
   auto mainHLayoutLeftPanel = make_shared<VLayout>("mainHLayoutLeftPanel");
   mainHLayout->addChild(mainHLayoutLeftPanel);

   //create scene tree panel
   {
      auto treeRoot = make_shared<TreeItem>("SceneRoot");
      sceneTree = make_shared<SceneTree>("sceneTree");
      sceneTree->setRoot(treeRoot);
      mainHLayoutLeftPanel->addChild(sceneTree);
      sceneTree->getTheme()->background.colorPrimary.set(ReyEngine::Colors::yellow);
   }

   //create widget tree panel
   {
      widgetTree = make_shared<WidgetTree>("widgetTree");
      mainHLayoutLeftPanel->addChild(widgetTree);
      widgetTree->getTheme()->background.colorPrimary.set(ReyEngine::Colors::green);
   }

   //create the (blank) workspace
   auto workspace = make_shared<Workspace>("Workspace");
   workspace->getTheme()->background.colorPrimary.set(COLORS::gray);
   mainHLayout->addChild(workspace);

   //create the right panel inspector
   inspector = make_shared<Inspector>("Inspector");
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