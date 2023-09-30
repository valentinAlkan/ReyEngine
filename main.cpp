#include <utility>
#include "DrawInterface.h"
#include "Application.h"
#include <iostream>
#include "Logger.h"
#include "ArgParse.h"
#include "TestWidgets.h"

using namespace std;
using namespace GFCSDraw;

int screenWidth = 1500;
int screenHeight = 800;

void updateFrame(float);

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
   Application::instance(); //initialize the application
   ArgParse args;
   args.defineArg(RuntimeArg("--loadScene", "help", 1, RuntimeArg::ArgType::OPTIONAL));
   args.parseArgs(argc, argv);

   shared_ptr<BaseWidget> root;
   auto optArg = args.getArg("loadScene");
   if (optArg){
      auto loadSceneArg = optArg.value();
      //open and test a scene file
      auto loadedScene = Scene::fromFile("./test/" + loadSceneArg->getParams()[0]);
      if (loadedScene){
         Application::printDebug() << "Got loaded file!" << endl;
         root = loadedScene.value()->getRoot();
      }
   }

   //create a root widget
   if (!root) {
      root = std::make_shared<PosTestWidget>("Root");
      auto child = std::make_shared<PosTestWidget>("Child");
      child->setPos(100, 100);
      root->addChild(child);
   }
   auto optWindow = Application::instance().createWindow("MainWindow", screenWidth, screenHeight, root, {Window::RESIZE});
//   auto s = StringProperty().toString();
   if (optWindow){
      auto mainWindow = optWindow.value();
      root->setProcess(true);
      mainWindow->exec();
      return 0;
   }

   return 1;
}