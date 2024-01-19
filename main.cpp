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
#include "Timer.hpp"
#include "Layout.hpp"
#include "Panel.hpp"
#include "Editor.h"
#include "Tree.h"
#include "Inspector.h"

using namespace std;
using namespace GFCSDraw;

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
   args.parseArgs(argc, argv);

   //create window (or don't idk)
   auto window = Application::instance().createWindow("MainWindow", screenWidth, screenHeight, {Window::RESIZE});
   if (!window){throw std::runtime_error("Something went horribly wrong! Please make a note of it.");}

   shared_ptr<BaseWidget> root = make_shared<Control>("Root", Rect<float>{0,0,0,0});
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

   else if (args.getArg("--inputPositionTest")){
      auto control = make_shared<Control>("MainControl", Rect<int>(40,40, 50,50));
      auto subcontrol = make_shared<Control>("SubControl", Rect<int>(100,100, 50,50));
      control->getTheme()->background.set(Style::Fill::SOLID);
      control->getTheme()->background.colorPrimary.set(COLORS::lightGray);

      auto renderSubControl = [subcontrol](){
         auto rect = subcontrol->getGlobalRect();
         GFCSDraw::drawRectangle(rect, COLORS::blue);
      };

      auto process = [&](){
         subcontrol->setRect({{control->globalToLocal(InputManager::getMousePos())},{50,50}});
      };

      control->addChild(subcontrol);
      subcontrol->setRenderCallback(renderSubControl);
      subcontrol->setProcessCallback(process);
      root = control;
   }

   else if (args.getArg("--renderTest")){
      root = make_shared<RootWidget>("Root", Rect<float> {0,0,0,0});
      auto textureTest = make_shared<TextureTestWidget>("TexTest", Rect<float> {0,0,100,100});
      root->addChild(textureTest);
   }

   else if (args.getArg("--labelTest")){
      root = make_shared<RootWidget>("Root", Rect<float> {0,0,0,0});
      auto testLabel = make_shared<Label>("Label", Rect<double>{0,0,50,50});
      root->addChild(testLabel);
//      testLabel->setOutlineType(Style::Outline::LINE);
//      testLabel->setBackgroundType(Theme::Outline::LINE);
   }


   else if (args.getArg("--scrollAreaTest")){
      root = make_shared<Control>("RootControl", Rect<int>(0,0, 2000, 2000));

      //add labels
      auto labelLayout = make_shared<HLayout>("labelLayout", Rect<int>(50,20,150,20));
      root->addChild(labelLayout);
      auto xlabel = make_shared<Label>("XLabel", Rect<int>());
      auto ylabel = make_shared<Label>("YLabel", Rect<int>());
      auto spacer = make_shared<Control>("spacer", Rect<int>());
      labelLayout->addChild(xlabel);
      labelLayout->addChild(ylabel);

      //add scroll area
      auto scrollArea = make_shared<ScrollArea>("ScrollArea",Rect<int>(50, 50, 500, 500));
      auto label1 = make_shared<Label>("ScrollAreaLabel1", Rect<int> {0,0,0,0});
      auto label2 = make_shared<Label>("ScrollAreaLabel2", Rect<int> {300,300,0,0});
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
      auto box = std::make_shared<Control>("OutlineControl", boxRect);
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
      auto rootControl = make_shared<Control>("RootControl", Rect<int>(0,0,2000,2000));
      root = rootControl;
      //add some children
      auto label1 = make_shared<Label>("Label1", Rect<int>(40,40,0,0));
      root->addChild(label1);
      auto label2 = make_shared<Label>("Label2", Rect<int>(40,300,0,0));
      root->addChild(label2);

      //draw the child bounding box
      auto drawBoundingBox = [&](){
         auto size = rootControl->getChildBoundingBox();
         auto mousePos = InputManager::getMousePos();
         GFCSDraw::drawRectangle({{0,0},size}, GFCSDraw::Colors::yellow);
         GFCSDraw::drawRectangle(label1->getRect(), GFCSDraw::Colors::green);
         GFCSDraw::drawRectangle(label2->getRect(), GFCSDraw::Colors::green);
         GFCSDraw::drawLine({label1->localToGlobal({0, 0}), mousePos}, COLORS::red);
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
      auto rootLayout = make_shared<VLayout>("Root", Rect<float>({0,0}, window->getSize()));
      root = rootLayout;
      auto mainPanel = make_shared<Panel>("MainPanel", Rect<int>());
      mainPanel->getTheme()->background.colorPrimary.set(GFCSDraw::Colors::red);
      root->addChild(mainPanel);
      mainPanel->setLayout<HLayout>();

      auto child1 = make_shared<Panel>("Child1", Rect<int>());
      auto child2 = make_shared<Panel>("Child2", Rect<int>());
//      child1->getTheme()->background.colorPrimary.set(GFCSDraw::Colors::blue);
//      child2->getTheme()->background.colorPrimary.set(GFCSDraw::Colors::green);
      mainPanel->addToLayout(child1);
      mainPanel->addToLayout(child2);

   }

   else if (args.getArg("--layoutTest")){
      Application::printDebug() << "Layout test!" << endl;
      root = make_shared<VLayout>("Root", Rect<float>({0,0}, window->getSize()));
      auto parent = root;
      for (int i=0;i<50;i++){
         std::shared_ptr<Layout> layout;
         if (i & 1){
            layout = make_shared<VLayout>("layout" + to_string(i), Rect<float>());
         } else {
            layout = make_shared<HLayout>("layout" + to_string(i), Rect<float>());
         }
         auto label1 = make_shared<Label>("label" + to_string(i) + "_1", Rect<float>());
         label1->setText("");
//         auto label2 = make_shared<Label>("label" + to_string(i) + "_2", Rect<float>());
         label1->getTheme()->background.set(Style::Fill::SOLID);
//         label2->getTheme()->background.set(Style::Fill::SOLID);
         label1->getTheme()->background.colorPrimary.set(ColorRGBA::random(254));
//         label2->getTheme()->background.colorPrimary.set(ColorRGBA::random(100));
         parent->addChild(layout);
         layout->addChild(label1);
//         layout->addChild(label2);
         parent = layout;

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
      root = make_shared<VLayout>("MainLayout", Rect<int>());

      //add a panel to the layout
      auto panel = make_shared<Panel>("Panel", Rect<int>());

      //add a slider so we can adjust the roundness dynamically
      auto slider = make_shared<Slider>("Slider", Rect<int>(40, 20, 100, 20), Slider::SliderType::HORIZONTAL);
      panel->addChild(slider);
      //add an hlayout to fill the slider
      auto sliderHLayout = make_shared<HLayout>("sliderLayout", slider->getRect().toSizeRect());
      slider->addChild(sliderHLayout);

      //add a name label to the slider
      auto fieldLabel = make_shared<Label>("FieldNameRoundness", Rect<int>());
      fieldLabel->setText("Roundness: ");
      //add a value label to the slider
      auto valueLabel = make_shared<Label>("roundnessLabel", Rect<int>());
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
      root = make_shared<Editor>("Editor", Rect<int>());
   }

   else if (args.getArg("--treeTest")){
      auto treeRoot = std::make_shared<TreeItem>("Root");
      auto tree = make_shared<Tree>("Tree", Rect<int>());
      tree->setRoot(treeRoot);
      root = tree;
      
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

   else if (args.getArg("--inspector")){
      //make a vlayout - put a widget at the top and the inspector below it
      root = make_shared<VLayout>("VLayout", Rect<int>());
      auto someWidget = make_shared<Label>("SomeWidget", GFCSDraw::Rect<int>());
      auto inspector = make_shared<Inspector>("Inspector", GFCSDraw::Rect<int>());
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

   //lock root to window size
//   auto resizeRoot = [&](const Window::WindowResizeEvent& event){
//      Application::printDebug() << "Running scene. Root's name is " << root->getName() << endl;
////      root->setRect({{0,0}, event.size});
//   };

   //panels
//   root->subscribe<Window::WindowResizeEvent>(window, resizeRoot);
   window->setRoot(root);
   root->setAnchoring(BaseWidget::Anchor::FILL);
   window->exec();
   return 0;
}