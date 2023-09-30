#include <utility>
#include "DrawInterface.h"
#include "Application.h"
#include <iostream>
#include "Logger.h"
#include "ArgParse.h"

using namespace std;
using namespace GFCSDraw;

int screenWidth = 1500;
int screenHeight = 800;

void updateFrame(float);


/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
class PosTestWidget : public BaseWidget {
public:
   PosTestWidget(string name)
   : BaseWidget(std::move(name), "PosTestWidget")
   , someString("someString")
   {}
   StringProperty someString;

   void render() const override {
      Vec2<int> pos = getGlobalPos();
      stringstream text;
      text << getName() << "\n";
      text << getPos() << "\n";
      text << "{" + to_string(pos.x) + "," + to_string(pos.y) + "}";
      _drawText(text.str(), {0,0}, 20, RED);
   }

   void _process(float dt) override{
      setPos(Window::getMousePos());
   }

   void registerProperties() override {
      registerProperty(someString);
   }
};

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
   ArgParse args;
   args.defineArg(RuntimeArg("positionalArg", "help"));
   args.defineArg(RuntimeArg("--testSceneFile"));
   args.parseArgs(argc, argv);

   if (args.getArg("testSceneFile")){
      //open and test a scene file
      auto loadedScene = Scene::fromFile("./test/test.scn");
      if (loadedScene){
         Application::printDebug() << "Got loaded file!" << endl;
      }
   }

   //create a root widget
   auto root = std::make_shared<PosTestWidget>("Root");
   auto child = std::make_shared<PosTestWidget>("Child");
   child->setPos(100,100);
   root->addChild(child);
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