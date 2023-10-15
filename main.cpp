#include <utility>
#include "DrawInterface.h"
#include "Application.h"
#include <iostream>
#include "Logger.h"
#include "ArgParse.h"
#include "TestWidgets.h"
#include "Label.hpp"
#include "Button.hpp"

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
      root = make_shared<Control>("Root");
      auto button = make_shared<PushButton>("Button");
      button->setPos(100, 100);
      root->addChild(button);
      auto label = make_shared<Label>("label");
      root->addChild(label);

      auto cb = [&](const std::shared_ptr<Event>& e){
         static int pushCount = 0;
         auto pbEvent = static_pointer_cast<PushButtonEvent>(e);
         if (pbEvent->down){
            label->setText(to_string(pushCount++));
         }
      };

      label->subscribe(button, EventType::EVENT_PUSHBUTTON, cb);
   }



   auto optWindow = Application::instance().createWindow("MainWindow", screenWidth, screenHeight, root, {Window::RESIZE});
   if (optWindow){
      auto mainWindow = optWindow.value();
      mainWindow->exec();
      return 0;
   }

   return 1;
}