#include "DrawInterface.h"
#include "ScrollArea.hpp"
#include "Application.h"
#include <iostream>
#include "Logger.h"
#include "ArgParse.h"
#include "Label.hpp"
#include "Button.hpp"
#include "RootWidget.hpp"
#include "TextureTestWidget.hpp"
#include "Slider.hpp"
#include "Timer.hpp"
#include "Layout.hpp"
#include "Panel.hpp"
#include "Editor.h"
#include "Tree.h"
#include "Inspector.h"
#include "Sprite.h"
#include "Canvas.hpp"

using namespace std;
using namespace ReyEngine;

int screenWidth = 1500;
int screenHeight = 800;

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
   SetTraceLogLevel(LOG_ERROR);
//   class EventTester : public EventSubscriber, public EventPublisher{
//
//   };
//   auto cb1 = [](const TestEvent1& testEvent){
//      cout << "My event ID is " << testEvent.eventId << endl;
//   };
//   auto cb2 = [](const TestEvent2& testEvent){
//      cout << "My event ID is " << testEvent.eventId << endl;
//   };
//   auto tester = std::make_shared<EventTester>();
//   tester->subscribe<TestEvent1>(tester, cb1);
//   tester->subscribe<TestEvent2>(tester, cb2);
//   TestEvent1 tevent1(tester);
//   TestEvent2 tevent2(tester);
//   tester->publish(tevent1);
//   tester->publish(tevent1);
//   tester->publish(tevent1);
//   tester->publish(tevent2);


   Application::instance(); //initialize the application
   ArgParse args;
   args.defineArg(RuntimeArg("--loadScene", "help", 1, RuntimeArg::ArgType::POSITIONAL));
   args.defineArg(RuntimeArg("--renderTest", "help", 0, RuntimeArg::ArgType::FLAG));
   args.defineArg(RuntimeArg("--scrollAreaTest", "help", 0, RuntimeArg::ArgType::FLAG));
   args.defineArg(RuntimeArg("--sliderTest", "help", 0, RuntimeArg::ArgType::FLAG));
   args.defineArg(RuntimeArg("--labelTest", "help", 0, RuntimeArg::ArgType::FLAG));
   args.defineArg(RuntimeArg("--saveLoadSceneTest", "Filename to save/load to", 1, RuntimeArg::ArgType::POSITIONAL));
   args.defineArg(RuntimeArg("--layoutTest", "Test layouts", 0, RuntimeArg::ArgType::FLAG));
   args.defineArg(RuntimeArg("--layoutTestBasic", "Basic Test layouts", 0, RuntimeArg::ArgType::FLAG));
   args.defineArg(RuntimeArg("--panelTest", "Test panel", 0, RuntimeArg::ArgType::FLAG));
   args.defineArg(RuntimeArg("--editor", "Editor", 0, RuntimeArg::ArgType::FLAG));
   args.defineArg(RuntimeArg("--treeTest", "TreeTest", 0, RuntimeArg::ArgType::FLAG));
   args.defineArg(RuntimeArg("--childBoundingBoxTest", "ChildBoundingBoxTest", 0, RuntimeArg::ArgType::FLAG));
   args.defineArg(RuntimeArg("--inputPositionTest", "InputPositionTest", 0, RuntimeArg::ArgType::FLAG));
   args.defineArg(RuntimeArg("--inspector", "InspectorTest", 0, RuntimeArg::ArgType::FLAG));
   args.defineArg(RuntimeArg("--spriteTest", "SpriteTest", 0, RuntimeArg::ArgType::FLAG));
   args.defineArg(RuntimeArg("--buttonTest", "PushButton usage example", 0, RuntimeArg::ArgType::FLAG));
   args.defineArg(RuntimeArg("--anchorTest", "Anchoring options test", 0, RuntimeArg::ArgType::FLAG));
   args.parseArgs(argc, argv);

   //create window (or don't idk)
   auto window = Application::instance().createWindow("MainWindow", screenWidth, screenHeight, {Window::RESIZE});
   if (!window){throw std::runtime_error("Something went horribly wrong! Please make a note of it.");}

   auto root = make_shared<Canvas>("Root");
   root->setAnchoring(BaseWidget::Anchor::FILL);
   auto argLoadScene = args.getArg("--loadScene");
   if (argLoadScene){
      auto loadSceneArg = argLoadScene.value();
      //open and test a scene file
      auto loadedScene = Scene::fromFile("./test/" + loadSceneArg->getParams()[0]);
      if (loadedScene){
         Application::printDebug() << "Got loaded file!" << endl;
         root->addChild(loadedScene.value()->getRoot());
      }
   }

   else if (args.getArg("--inputPositionTest")){
      auto control = make_shared<Control>("MainControl");
      control->setRect(Rect<int>(40,40, 50,50));
      auto subcontrol = make_shared<Control>("SubControl");
      subcontrol->setRect(Rect<int>(100,100, 50,50));
      control->getTheme()->background.set(Style::Fill::SOLID);
      control->getTheme()->background.colorPrimary.set(COLORS::lightGray);

      auto renderSubControl = [subcontrol](){
         auto rect = subcontrol->getGlobalRect();
         ReyEngine::drawRectangle(rect, COLORS::blue);
      };

      auto process = [&](){
         subcontrol->setRect({{control->globalToLocal(InputManager::getMousePos())},{50,50}});
      };

      control->addChild(subcontrol);
      subcontrol->setRenderCallback(renderSubControl);
      subcontrol->setProcessCallback(process);
      root->addChild(control);
   }

   else if (args.getArg("--renderTest")){
      root->addChild(make_shared<RootWidget>("Root"));
      root->setRect({0,0,0,0});
      auto textureTest = make_shared<TextureTestWidget>("TexTest");
      textureTest->setRect({0,0,100,100});
      root->addChild(textureTest);
   }

   else if (args.getArg("--labelTest")){
      root->addChild(make_shared<RootWidget>("Root"));
      auto testLabel = make_shared<Label>("Label");
      testLabel->setRect({0,0,50,50});  
      root->addChild(testLabel);
//      testLabel->setOutlineType(Style::Outline::LINE);
//      testLabel->setBackgroundType(Theme::Outline::LINE);
   }


   else if (args.getArg("--scrollAreaTest")){
      root->addChild(make_shared<Control>("Control"));
      root->setRect({0, 0, 2000, 2000});

      //add labels
      auto labelLayout = make_shared<HLayout>("labelLayout");
      labelLayout->setRect(Rect<int>(50,20,150,20));
      root->addChild(labelLayout);
      auto xlabel = make_shared<Label>("XLabel");
      auto ylabel = make_shared<Label>("YLabel");
      auto spacer = make_shared<Control>("spacer");
      labelLayout->addChild(xlabel);
      labelLayout->addChild(ylabel);

      //add scroll area
      auto scrollArea = make_shared<ScrollArea>("ScrollArea");
      scrollArea->setRect(Rect<int>(50, 50, 500, 500));
      auto label1 = make_shared<Label>("ScrollAreaLabel1");
      auto label2 = make_shared<Label>("ScrollAreaLabel2");
      label2->setRect({300,300,0,0});
      label1->setText("Hello from the upper left!");
      label2->setText("Hello from the bottom right!");
      scrollArea->addChild(label1);
      scrollArea->addChild(label2);
      scrollArea->getTheme()->background.colorPrimary.set(COLORS::red);

      //draw a box around the scroll area
      auto boxRect = scrollArea->getRect();
      boxRect.x -= 1;
      boxRect.y -= 1;
      boxRect.width += 1;
      boxRect.height += 1;
      auto box = std::make_shared<Control>("OutlineControl");
      box->setRect(boxRect);
      box->getTheme().get()->background.set(Style::Fill::SOLID);
      box->getTheme().get()->background.colorPrimary.set({125, 125, 125, 127});
      root->addChild(box);

      root->addChild(scrollArea);
//
      auto displaySize = [&](const BaseWidget::WidgetResizeEvent&){
         xlabel->setText(scrollArea->getWidth());
         ylabel->setText(scrollArea->getHeight());
         box->setRect(scrollArea->getRect());
      };
      labelLayout->subscribe<BaseWidget::WidgetResizeEvent>(scrollArea, displaySize);
      scrollArea->setInEditor(true);
   }

   else if (args.getArg("--childBoundingBoxTest")){
      auto rootControl = make_shared<Control>("RootControl");
      rootControl->setRect({0,0,2000,2000});
      root->addChild(rootControl);
      //add some children
      auto label1 = make_shared<Label>("Label1");
      label1->setRect(Rect<int>(40,40,0,0));
      root->addChild(label1);
      auto label2 = make_shared<Label>("Label2");
      label2->setRect(Rect<int>(40,300,0,0));
      root->addChild(label2);

      //draw the child bounding box
      auto drawBoundingBox = [&](){
         auto size = rootControl->getChildBoundingBox();
         auto mousePos = InputManager::getMousePos();
         ReyEngine::drawRectangle({{0,0},size}, ReyEngine::Colors::yellow);
         ReyEngine::drawRectangle(label1->getRect(), ReyEngine::Colors::green);
         ReyEngine::drawRectangle(label2->getRect(), ReyEngine::Colors::green);
         ReyEngine::drawLine({label1->localToGlobal({0, 0}), mousePos}, COLORS::red);
      };
      rootControl->setRenderCallback(drawBoundingBox);

      auto process = [&](){
         auto globalPos = InputManager::getMousePos();
         // reposition label
         auto newPos = label1->getParent().lock()->globalToLocal(globalPos);
         label1->setRect({newPos, label1->getSize()});
      };
      rootControl->setProcessCallback(process);

   }


   else if (args.getArg("--sliderTest")){
      root->addChild(make_shared<Control>("Root"));
      auto label = make_shared<Label>("Label");

      auto vslider = make_shared<Slider>("Vslider", Slider::SliderType::VERTICAL);
      vslider->setRect({200,100,20,100});
      auto hslider = make_shared<Slider>("Hslider", Slider::SliderType::HORIZONTAL);
      hslider->setRect({250,100,100,20});

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

      //move timer
      auto timerCb = [&](const Timer::TimeoutEvent& e){
         auto pct = window->getMousePct();
         hslider->setSliderPct(pct.x);
         vslider->setSliderPct(pct.y);
      };
      auto timer = SystemTime::newTimer(std::chrono::milliseconds(1000));
      label->subscribe<Timer::TimeoutEvent>(timer, timerCb);
      auto timerProperty = make_shared<TimerProperty>("timer", timer);
      label->moveProperty(timerProperty);

   }

   else if (args.getArg("--saveLoadSceneTest")){
//      auto
   }

   else if (args.getArg("--layoutTestBasic")){
      Application::printDebug() << "Layout test basic!" << endl;
      auto rootLayout = make_shared<VLayout>("Root");
      rootLayout->setRect(Rect<float>({0,0}, window->getSize()));
      root->addChild(rootLayout);
      auto mainPanel = make_shared<Panel>("MainPanel");
      mainPanel->getTheme()->background.colorPrimary.set(ReyEngine::Colors::red);
      root->addChild(mainPanel);
      mainPanel->setLayout<HLayout>();

      auto child1 = make_shared<Panel>("Child1");
      auto child2 = make_shared<Panel>("Child2");
//      child1->getTheme()->background.colorPrimary.set(ReyEngine::Colors::blue);
//      child2->getTheme()->background.colorPrimary.set(ReyEngine::Colors::green);
      mainPanel->addToLayout(child1);
      mainPanel->addToLayout(child2);

   }

   else if (args.getArg("--layoutTest")){
      Application::printDebug() << "Layout test!" << endl;
      auto rootChild = make_shared<VLayout>("RootChild");
      rootChild->setRect(Rect<float>({0,0}, window->getSize()));
      root->addChild(rootChild);
      for (int i=0;i<50;i++){
         std::shared_ptr<Layout> layout;
         if (i & 1){
            layout = make_shared<VLayout>("layout" + to_string(i));
         } else {
            layout = make_shared<HLayout>("layout" + to_string(i));
         }
         auto label1 = make_shared<Label>("label" + to_string(i) + "_1");
         label1->setText("");
//         auto label2 = make_shared<Label>("label" + to_string(i) + "_2", Rect<float>());
         label1->getTheme()->background.set(Style::Fill::SOLID);
//         label2->getTheme()->background.set(Style::Fill::SOLID);
         label1->getTheme()->background.colorPrimary.set(ColorRGBA::random(254));
//         label2->getTheme()->background.colorPrimary.set(ColorRGBA::random(100));
         root->addChild(layout);
         layout->addChild(label1);
//         layout->addChild(label2);
         root->addChild(layout);

         //connect to resize
//         auto resizeCB = [](const BaseWidget::WidgetResizeEvent& event){
//            auto label = event.subscriber->toBaseWidget()->toType<Label>();
//            label->setText(event.size);
//         };
//         label1->subscribe<BaseWidget::WidgetResizeEvent>(parent, resizeCB);
//         label2->subscribe<BaseWidget::WidgetResizeEvent>(parent, resizeCB);
      }
   }

   else if (args.getArg("--panelTest")){
      root->addChild(make_shared<VLayout>("MainLayout"));

      //add a panel to the layout
      auto panel = make_shared<Panel>("Panel");

      //add a slider so we can adjust the roundness dynamically
      auto slider = make_shared<Slider>("Slider", Slider::SliderType::HORIZONTAL);
      slider->setRect(Rect<int>(40, 20, 100, 20));
      panel->addChild(slider);
      //add an hlayout to fill the slider
      auto sliderHLayout = make_shared<HLayout>("sliderLayout");
      sliderHLayout->setRect(slider->getRect().toSizeRect());
      slider->addChild(sliderHLayout);

      //add a name label to the slider
      auto fieldLabel = make_shared<Label>("FieldNameRoundness");
      fieldLabel->setText("Roundness: ");
      //add a value label to the slider
      auto valueLabel = make_shared<Label>("roundnessLabel");
      valueLabel->setText(0, 3);

      sliderHLayout->addChild(fieldLabel);
      sliderHLayout->addChild(valueLabel);
      sliderHLayout->setWidth(fieldLabel->getWidth() + valueLabel->getWidth());

      //set the layout ratio so everything can be displayed nicely
      sliderHLayout->childScales.set(0, 1.5);

      //resize slider so it fits all our text
      slider->setWidth(sliderHLayout->getWidth());

      auto updateLabel = [panel, valueLabel](const Slider::SliderValueChangedEvent& event){
         double newRound = (float)event.value/100.0;
         valueLabel->setText(newRound, 3);
         panel->getTheme()->roundness.set(newRound);
      };
      valueLabel->subscribe<Slider::SliderValueChangedEvent>(slider, updateLabel);
      root->addChild(panel);
   }

   else if (args.getArg("--editor")){
      root->addChild(make_shared<Editor>("Editor"));
   }

   else if (args.getArg("--treeTest")){
      auto treeRoot = std::make_shared<TreeItem>("Root");
      auto tree = make_shared<Tree>("Tree");
      tree->setRoot(treeRoot);
      root->addChild(tree);
      
      for (auto item : {
         std::make_shared<TreeItem>("Child1"),
         std::make_shared<TreeItem>("Child2"),
         std::make_shared<TreeItem>("Child3")
               }) {
         treeRoot->push_back(item);
      }
      for (auto item : {
            std::make_shared<TreeItem>("GrandChild01"),
            std::make_shared<TreeItem>("GrandChild02"),
            std::make_shared<TreeItem>("GrandChild03")
      }) {
         treeRoot->getChildren()[0]->push_back(item);
      }

      for (auto item : {
            std::make_shared<TreeItem>("GreatGrandChild01"),
            std::make_shared<TreeItem>("GreatGrandChild02"),
            std::make_shared<TreeItem>("GreatGrandChild03")
      }) {
         treeRoot->getChildren()[0]->getChildren()[0]->push_back(item);
      }

      for (auto item : {
            std::make_shared<TreeItem>("GreatGreatGrandChild01"),
            std::make_shared<TreeItem>("GreatGreatGrandChild02"),
            std::make_shared<TreeItem>("GreatGreatGrandChild03")
      }) {
         treeRoot->getChildren()[0]->getChildren()[0]->getChildren()[0]->push_back(item);
      }


      for (auto item : {
            std::make_shared<TreeItem>("GrandChild11"),
            std::make_shared<TreeItem>("GrandChild12"),
            std::make_shared<TreeItem>("GrandChild13")
      }) {
         treeRoot->getChildren()[1]->push_back(item);
      }

      for (auto item : {
            std::make_shared<TreeItem>("GrandChild21"),
            std::make_shared<TreeItem>("GrandChild22"),
            std::make_shared<TreeItem>("GrandChild23")
      }) {
         treeRoot->getChildren()[2]->push_back(item);
      }

      //iterate over the tree
      for (const auto& item : *tree){
         Application::printDebug() << item.getText() << endl;
      }

      tree->setHideRoot(true);
   }

   else if (args.getArg("--inputPositionTest")){

   }

   else if (args.getArg("--spriteTest")){
      //make sure you've installed any sprite resources in the CMake file or they will not be visible to the engine.
      // The executable runs out of the build directory and not the src directory.
      // use install_file() in the cmake
      root->addChild(make_shared<Control>("Control"));
      auto spriteSheet = make_shared<Sprite>("SpriteSheet"); //either pass in a rect or do fit texture later
      spriteSheet->setTexture("test\\characters.png"); //if no rect passed in, region = texture size
      spriteSheet->fitTexture();
      root->addChild(spriteSheet);

      auto animatedSprite = make_shared<Sprite>("AnimatedSpriteSheet");
      animatedSprite->setRect(Rect<int>(550,0,0,0));
      animatedSprite->setTexture("test\\characters.png");
      animatedSprite->fitTexture();
      animatedSprite->scale(Vec2<float>(5,5));
      root->addChild(animatedSprite);

   }

   else if (args.getArg("--buttonTest")) {
      root->addChild(make_shared<Control>("root"));
      auto label = make_shared<Label>("Label");
      label->setRect(Rect<int>(50,650,1000,50));
      root->addChild(label);
      auto vlayout = make_shared<VLayout>("VLayout");
      vlayout->setRect({100, 100, 200, 250});
      root->addChild(vlayout);

      //callback lambda(s)
      auto cb = [&](const Event<BaseButton::ButtonPressEvent>& event){
         auto senderName = event.publisher->toBaseWidget()->getName();
         label->setText(senderName);
         //you can do whatever else here
      };

      auto cbSecret = [&](const Event<BaseButton::ButtonPressEvent>& event){
         auto senderName = event.publisher->toBaseWidget()->getName();
         label->setText(senderName + " is the secret button! You win it all!");
         //you can do whatever else here
      };

      for (int i=0; i<5; i++) {
         static constexpr int secret = 2;
         auto button = make_shared<PushButton>("PushButton" + to_string(i));
         vlayout->addChild(button);
         //Subscribers don't have to have any relationship with the callbacks they call. In this example,
         // the vlayout is subscribing to the buttons' pushbutton events. However, the callback has nothing
         // to do with the vlayout itself. It's just a lambda. That's just a cool feature of this engine!
         //Callbacks are also typed - every event emits an event, and any handlers that wish to intercept it
         // must have the same signature.
         if (button->getName() == "PushButton" + to_string(secret)) {
            vlayout->subscribe<BaseButton::ButtonPressEvent>(button, cbSecret);
         } else {
            vlayout->subscribe<BaseButton::ButtonPressEvent>(button, cb);
         }
      }

      auto cbExit = [&](const Event<BaseButton::ButtonPressEvent>& event){
         //quit (crashes atm but w/e)
         Application::exit(Application::ExitReason::CLEAN);
      };

      auto exitButton = make_shared<PushButton>("Exit");
      vlayout->addChild(exitButton);
      exitButton->subscribe<BaseButton::ButtonPressEvent>(exitButton, cbExit);

   }

   else if (args.getArg("--anchorTest")){
       auto vLayout = make_shared<VLayout>("VLayout", Rect<int>({0, 0}, window->getSize()));

       //create the 3 HLayouts
       auto hLayoutTop = make_shared<HLayout>("TopHLayout", Rect<int>());
       hLayoutTop->setSize({vLayout->getWidth(), vLayout->getHeight() / 4});
       auto hLayoutMiddle = make_shared<HLayout>("MiddleHLayout", Rect<int>());
       hLayoutMiddle->setSize({vLayout->getWidth(), vLayout->getHeight() / 4});
       auto hLayoutBottom = make_shared<HLayout>("BottomHLayout", Rect<int>());
       hLayoutBottom->setSize({vLayout->getWidth(), vLayout->getHeight() / 4});
       auto hLayoutFill = make_shared<HLayout>("FillHLayout", Rect<int>());
       hLayoutBottom->setSize({vLayout->getWidth(), vLayout->getHeight() / 4});

       vLayout->addChild(hLayoutTop);
       vLayout->addChild(hLayoutMiddle);
       vLayout->addChild(hLayoutBottom);
       vLayout->addChild(hLayoutFill);

       //create test labels for all anchoring
       auto noneLabel = make_shared<Label>("NoneLabel", Rect<int>());
       noneLabel->setText("None");
       noneLabel->setAnchoring(BaseWidget::Anchor::NONE);
       hLayoutTop->addChild(noneLabel);

       auto leftLabel = make_shared<Label>("LeftLabel", Rect<int>());
       leftLabel->setText("Left");
       leftLabel->setAnchoring(BaseWidget::Anchor::LEFT);
       hLayoutTop->addChild(leftLabel);

       auto rightLabel = make_shared<Label>("RightLabel", Rect<int>());
       rightLabel->setText("Right");
       rightLabel->setAnchoring(BaseWidget::Anchor::RIGHT);
       hLayoutTop->addChild(rightLabel);

       auto topLabel = make_shared<Label>("TopLabel", Rect<int>());
       topLabel->setText("Top");
       topLabel->setAnchoring(BaseWidget::Anchor::TOP);
       hLayoutTop->addChild(topLabel);

       auto bottomLabel = make_shared<Label>("BottomLabel", Rect<int>());
       bottomLabel->setText("Bottom");
       bottomLabel->setAnchoring(BaseWidget::Anchor::BOTTOM);
       hLayoutMiddle->addChild(bottomLabel);

       auto bottomLeftLabel = make_shared<Label>("BottomLeftLabel", Rect<int>());
       bottomLeftLabel->setText("Bottom Left");
       bottomLeftLabel->setAnchoring(BaseWidget::Anchor::BOTTOM_LEFT);
       hLayoutMiddle->addChild(bottomLeftLabel);

       auto bottomRightLabel = make_shared<Label>("BottomRightLabel", Rect<int>());
       bottomRightLabel->setText("Bottom Right");
       bottomRightLabel->setAnchoring(BaseWidget::Anchor::BOTTOM_RIGHT);
       hLayoutMiddle->addChild(bottomRightLabel);

       auto topLeftLabel = make_shared<Label>("TopLeftLabel", Rect<int>());
       topLeftLabel->setText("Top Left");
       topLeftLabel->setAnchoring(BaseWidget::Anchor::TOP_LEFT);
       hLayoutMiddle->addChild(topLeftLabel);

       auto topRightLabel = make_shared<Label>("TopRightLabel", Rect<int>());
       topRightLabel->setText("Top Right");
       topRightLabel->setAnchoring(BaseWidget::Anchor::TOP_RIGHT);
       hLayoutBottom->addChild(topLabel);

       auto fillButton = make_shared<PushButton>("FillLabel", Rect<int>());
       fillButton->setText("Fill");
       fillButton->setAnchoring(BaseWidget::Anchor::FILL);
       hLayoutFill->addChild(fillButton);

       auto centerLabel = make_shared<Label>("CenterLabel", Rect<int>());
       centerLabel->setText("Center");
       centerLabel->setAnchoring(BaseWidget::Anchor::CENTER);
       hLayoutBottom->addChild(centerLabel);

       root->addChild(vLayout);
   }

   else if (args.getArg("--inspector")){
      //make a vlayout - put a widget at the top and the inspector below it
      root->addChild(make_shared<VLayout>("VLayout"));
      auto someWidget = make_shared<Label>("SomeWidget");
      auto inspector = make_shared<Inspector>("Inspector");
      someWidget->getTheme()->background.set(Style::Fill::SOLID);
      someWidget->getTheme()->background.colorPrimary.set(Colors::blue);
      root->addChild(someWidget);
      root->addChild(inspector);
      inspector->inspect(someWidget);
   }

   else {
      cout << args.getDocString() << endl;
      return 0;
   }

   window->setRoot(root);
   root->setAnchoring(BaseWidget::Anchor::FILL);
   window->exec();
   return 0;
}