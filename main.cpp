#include <utility>
#include "DrawInterface.h"
#include "ScrollArea.hpp"
#include "Application.h"
#include <iostream>
#include "Logger.h"
#include "ArgParse.h"
#include "TestWidgets.h"
#include "Label.hpp"
#include "Button.hpp"
#include "RootWidget.hpp"
#include "TextureTestWidget.hpp"
#include "Slider.hpp"

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
   class EventTester : public EventSubscriber, public EventPublisher{

   };
   auto cb1 = [](const TestEvent1& testEvent){
      cout << "My event ID is " << testEvent.eventId << endl;
   };
   auto cb2 = [](const TestEvent2& testEvent){
      cout << "My event ID is " << testEvent.eventId << endl;
   };
   auto tester = std::make_shared<EventTester>();
   tester->subscribe<TestEvent1>(tester, cb1);
   tester->subscribe<TestEvent2>(tester, cb2);
   TestEvent1 tevent1(tester);
   TestEvent2 tevent2(tester);
   tester->publish(tevent1);
   tester->publish(tevent1);
   tester->publish(tevent1);
   tester->publish(tevent2);


   Application::instance(); //initialize the application
   ArgParse args;
   args.defineArg(RuntimeArg("--loadScene", "help", 1, RuntimeArg::ArgType::OPTIONAL));
   args.defineArg(RuntimeArg("--renderTest", "help", 0, RuntimeArg::ArgType::FLAG));
   args.defineArg(RuntimeArg("--scrollArea", "help", 0, RuntimeArg::ArgType::FLAG));
   args.defineArg(RuntimeArg("--sliderTest", "help", 0, RuntimeArg::ArgType::FLAG));
   args.parseArgs(argc, argv);

   shared_ptr<BaseWidget> root;
   auto argLoadScene = args.getArg("--loadScene");
   if (argLoadScene){
      auto loadSceneArg = argLoadScene.value();
      //open and test a scene file
      auto loadedScene = Scene::fromFile("./test/" + loadSceneArg->getParams()[0]);
      if (loadedScene){
         Application::printDebug() << "Got loaded file!" << endl;
         root = loadedScene.value()->getRoot();
      }
   }

   if (args.getArg("--renderTest")){
      root = make_shared<RootWidget>("Root", Rect<float> {0,0,0,0});
      auto textureTest = make_shared<TextureTestWidget>("TexTest", Rect<float> {0,0,100,100});
      root->addChild(textureTest);
   }


   if (args.getArg("--scrollArea")){
      root = make_shared<ScrollArea>("RootScrollArea", Rect<float> {50,50,200,200});
      auto label = make_shared<Label>("ScrollAreaLabel", Rect<float> {0,0,0,0});
      root->addChild(label);
   }

   if (args.getArg("--sliderTest")){
      root = make_shared<Control>("Root", Rect<float> {0,0,0,0});
      auto label = make_shared<Label>("Label", Rect<float> {0,0,0,0});

      auto vslider = make_shared<Slider>("Vslider", Rect<float>{200,100,20,100}, Slider::SliderType::VERTICAL);
      auto hslider = make_shared<Slider>("Hslider", Rect<float>{250,100,100,20}, Slider::SliderType::HORIZONTAL);

      root->addChild(label);
      root->addChild(vslider);
      root->addChild(hslider);

      auto labelMoveX = [&](const Slider::SliderValueChangedEvent& event){
         auto slider = event.publisher->toBaseWidget()->toType<Slider>();
         label->setPos(Vec2<int>(0,screenWidth-label->getRect().width).lerp(slider->getSliderPct()), label->getPos().y);
      };

      auto labelMoveY = [&](const Slider::SliderValueChangedEvent& event){
         auto slider = event.publisher->toBaseWidget()->toType<Slider>();
         label->setPos(label->getPos().x, Vec2<int>(0,screenHeight-label->getRect().height).lerp(slider->getSliderPct()));
      };
      label->subscribe<Slider::SliderValueChangedEvent>(hslider, labelMoveX);
      label->subscribe<Slider::SliderValueChangedEvent>(vslider, labelMoveY);
   }

   // default functionality
   //create a root widget
   if (!root) {
      root = make_shared<RootWidget>("Root", Rect<float> {0,0,0,0});
      auto button = make_shared<PushButton>("Button", Rect<float> {0,0,0,0});
      button->setPos(100, 100);
      root->addChild(button);
      auto label = make_shared<Label>("label", Rect<float> {0,0,0,0});

//      auto cb = [&](const BaseEvent& e){
//         static int pushCount = 0;
//         auto& pbEvent = e.toType<PushButtonEvent>();
//         if (pbEvent.down){
//            label->setText(to_string(pushCount++));
//         }
//      };

//      label->subscribe(button, PushButtonEvent::EVENT_PUSHBUTTON, cb);

      //create a scroll area
      auto scrollArea = make_shared<ScrollArea>("ScrollArea", Rect<float> {0,0,0,0});
      scrollArea->setRect({0, 0, 500,500});
      root->addChild(scrollArea);
      scrollArea->addChild(label);
   }


   auto optWindow = Application::instance().createWindow("MainWindow", screenWidth, screenHeight, root, {Window::RESIZE});
   if (optWindow){
      auto mainWindow = optWindow.value();
      mainWindow->exec();
      return 0;
   }

   return 1;
}