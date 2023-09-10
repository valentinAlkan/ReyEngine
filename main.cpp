#include <utility>
#include "DrawInterface.h"
#include "Application.h"
#include <iostream>

using namespace std;
using namespace GFCSDraw;

int screenWidth = 1500;
int screenHeight = 800;

void updateFrame(float);


/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
class TestWidget : public BaseWidget {
public:
   TestWidget(std::string name, std::shared_ptr<BaseWidget> parent = nullptr)
         : BaseWidget(std::move(name), std::move(parent)) {}

   void render() const override {
      auto center = getScreenCenter();
      std::string msg = "Screen cetner is {" + std::to_string(center.x) + "," + std::to_string(center.y) + "}";
      drawTextRelative(msg, Vec2<int>(0, 0), 20, LIGHTGRAY);
      drawTextCentered("Congrats! You created your first window!", center, 20, BLACK);
      drawTextRelative("Some text", Vec2<int>(10, 10), 20, LIGHTGRAY);
   }
};

class GlobalPosTestWidget : public BaseWidget {
public:
   GlobalPosTestWidget(string name, shared_ptr<BaseWidget> parent = nullptr)
   : BaseWidget(std::move(name), std::move(parent)){}

   void render() const override {
      Vec2<int> pos = getPos();
      string text = "{" + to_string(pos.x) + "," + to_string(pos.y) + "}";
      _drawText(text, {0,0}, 20, RED);
   }

   void _process(float dt) override{
      setPos(Window::getMousePos());
   }
};

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
int main()
{
   //create a root widget
   auto root = std::make_shared<GlobalPosTestWidget>("GlobalPos");
   root->setPos({100, 100});
   auto mainWindow = Application::instance().createWindow("MainWindow", screenWidth, screenHeight, root, {Window::RESIZE});
   root->setProcess(true);
   mainWindow->exec();
   return 0;
}