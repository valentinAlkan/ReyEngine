#include "DrawInterface.h"
#include "ScrollArea.hpp"
#include "Application.h"
#include <iostream>
#include "Logger.h"
#include "ArgParse.h"
#include "Label.hpp"
#include "Button.h"
#include "RootWidget.hpp"
#include "TextureTestWidget.hpp"
#include "Slider.hpp"
#include "Timer.hpp"
#include "Layout.h"
#include "Panel.hpp"
#include "Editor.h"
#include "Tree.h"
#include "Inspector.h"
#include "Sprite.h"
#include "Canvas.h"
#include "TabContainer.h"
#include "ComboBox.h"
#include "LineEdit.h"
#include "Config.h"
#include "XML.h"
#include "TileMap.h"

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
   args.defineArg(RuntimeArg("--xmlTest", "XML test", 0, RuntimeArg::ArgType::FLAG));
   args.defineArg(RuntimeArg("--layoutTest", "Test layouts", 0, RuntimeArg::ArgType::FLAG));
   args.defineArg(RuntimeArg("--configTest", "Config file test", 0, RuntimeArg::ArgType::FLAG));
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
   args.defineArg(RuntimeArg("--marginsTest", "Layout margins test", 0, RuntimeArg::ArgType::FLAG));
   args.defineArg(RuntimeArg("--hoverTest", "Hovering test", 0, RuntimeArg::ArgType::FLAG));
   args.defineArg(RuntimeArg("--tabContainerTest", "Tab container test", 0, RuntimeArg::ArgType::FLAG));
   args.defineArg(RuntimeArg("--relativeMotionTest", "Relative location movement test", 0, RuntimeArg::ArgType::FLAG));
   args.defineArg(RuntimeArg("--drawTest", "Test various drawing functions", 0, RuntimeArg::ArgType::FLAG));
   args.defineArg(RuntimeArg("--comboBoxTest", "Combo box test", 0, RuntimeArg::ArgType::FLAG));
   args.defineArg(RuntimeArg("--tileMapTest", "Config file test", 0, RuntimeArg::ArgType::FLAG));
   args.defineArg(RuntimeArg("--lineEditTest", "Line edit test", 0, RuntimeArg::ArgType::FLAG));
   args.parseArgs(argc, argv);

   //create window (or don't idk)
   auto window = Application::instance().createWindow("MainWindow", screenWidth, screenHeight, {Window::RESIZE});
   if (!window){throw std::runtime_error("Something went horribly wrong! Please make a note of it.");}

   auto root = window->getCanvas();
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

   else if (args.getArg("--drawTest")){
      auto control = make_shared<Control>("Drawbox");
      control->setAnchoring(BaseWidget::Anchor::FILL);
      root->addChild(control);

      auto drawcb = [control](){
         ReyEngine::drawRectangle({10,10,50,50}, Colors::red);
         float startang = 0;
         static int endang = 0;
         ReyEngine::drawCircleSectorLines({{100, 100}, 50, startang, (float) (endang++ % 360)}, Colors::blue, 20);
      };

      control->setRenderCallback(drawcb);
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

      auto process = [&](float dt){
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
      auto boxBounder = make_shared<Control>("BoxBounder");
      boxBounder->setRect({0, 0, 2000, 2000});
      root->addChild(boxBounder);
      //add some children
      auto label1 = make_shared<Label>("Label1");
      label1->setPos(Pos<int>(40,40));
      boxBounder->addChild(label1);
      auto label2 = make_shared<Label>("Label2");
      label2->setPos(Pos<int>(40,300));
      boxBounder->addChild(label2);

      //draw the child bounding box
      auto drawBoundingBox = [&](){
         auto size = boxBounder->getChildBoundingBox();
         auto mousePos = InputManager::getMousePos();
         ReyEngine::drawRectangle({{0,0},size}, ReyEngine::Colors::yellow);
         ReyEngine::drawRectangle(label1->getRect(), ReyEngine::Colors::green);
         ReyEngine::drawRectangle(label2->getRect(), ReyEngine::Colors::green);
         ReyEngine::drawLine({{0,0}, mousePos}, 1, COLORS::red);
      };
      boxBounder->setRenderCallback(drawBoundingBox);

      auto process = [&](float dt){
         auto globalPos = InputManager::getMousePos();
         // reposition label
         auto newPos = label1->getParent().lock()->globalToLocal(globalPos);
         label1->setRect({newPos, label1->getSize()});
      };
      boxBounder->setProcessCallback(process);

   }


   else if (args.getArg("--sliderTest")){
      auto inputFilter = make_shared<Control>("InputFilter");
      inputFilter->setAnchoring(BaseWidget::Anchor::FILL);
      root->addChild(inputFilter);
      auto control = make_shared<Control>("Control");
      control->setSize({100, 100});
      control->getTheme()->background.colorPrimary = Colors::red;

      auto vslider = make_shared<Slider>("Vslider", Slider::SliderType::VERTICAL);
      vslider->setRect({200,100,20,100});
      auto hslider = make_shared<Slider>("Hslider", Slider::SliderType::HORIZONTAL);
      hslider->setRect({250,100,100,20});

      inputFilter->addChild(control);
      inputFilter->addChild(vslider);
      inputFilter->addChild(hslider);

      bool down = false;
      Pos<int> offset;

      auto cbInput = [&](const InputEvent& event, const std::optional<UnhandledMouseInput>& mouse) -> Handled {
         switch(event.eventId){
            case InputEventMouseButton::getUniqueEventId(): {
               auto mbEvent = event.toEventType<InputEventMouseButton>();
               if (mbEvent.isDown && mouse->isInside) {
                  down = true;
               } else if (!mbEvent.isDown){
                  down = false;
               }
               return true;
            }
            case InputEventMouseMotion::getUniqueEventId(): {
               auto& mmEvent = event.toEventType<InputEventMouseMotion>();
               auto mouseLocal = mouse->localPos;
               if (down) {
                  auto xpct = 1-(((double)inputFilter->getRect().width - (double)control->getPos().x) / (double)inputFilter->getRect().width);
                  auto ypct = 1-(((double)inputFilter->getRect().height - (double)control->getPos().y) / (double)inputFilter->getRect().height);
                  if (xpct < 0) xpct = 0;
                  if (xpct > 1) xpct = 1;
                  if (ypct < 0) ypct = 0;
                  if (ypct > 1) ypct = 1;
                  mouseLocal = mouseLocal.clamp(Vec2<int>(0,0), inputFilter->getSize());
                  control->setPosRelative(mouseLocal, offset);
                  hslider->setSliderPct(xpct, true);
                  vslider->setSliderPct(ypct, true);
                  return true;
               } else {
                  offset = mouseLocal - control->getPos();
                  return true;
               }
               break;
            }
         }
         return false;
      };
      inputFilter->setUnhandledInputCallback(cbInput);

      //connect to slider events
      auto hSlidercb = [&](const Slider::SliderValueChangedEvent& event){
         control->setPos({(int)((double) inputFilter->getWidth() * event.pct), control->getPos().y});
      };
      inputFilter->subscribe<Slider::SliderValueChangedEvent>(hslider, hSlidercb);

      auto vSlidercb = [&](const Slider::SliderValueChangedEvent& event){
         control->setPos({control->getPos().x, (int)((double) inputFilter->getHeight() * event.pct)});
      };
      inputFilter->subscribe<Slider::SliderValueChangedEvent>(vslider, vSlidercb);

   }

   else if (args.getArg("--saveLoadSceneTest")){
//      auto
   }

   else if (args.getArg("--layoutTestBasic")){
      Application::printDebug() << "Layout test basic!" << endl;
      auto mainVLayout = make_shared<VLayout>("MainVLayout");
      mainVLayout->setAnchoring(BaseWidget::Anchor::FILL);
      root->addChild(mainVLayout);

      auto control1 = make_shared<Control>("Control1");
      auto control2 = make_shared<Control>("Control2");
      auto layout3 = make_shared<HLayout>("Layout3");
      control1->getTheme()->background = Style::Fill::SOLID;
      control1->getTheme()->background.colorPrimary.set(ReyEngine::Colors::red);
      control2->getTheme()->background = Style::Fill::SOLID;
      control2->getTheme()->background.colorPrimary.set(ReyEngine::Colors::green);
      mainVLayout->addChild(control1);
      mainVLayout->addChild(control2);
      mainVLayout->addChild(layout3);

      //split layout 3
      for (int i=0; i<4; i++) {
          auto subcontrol = make_shared<Control>("SubControl" + to_string(i));
          subcontrol->getTheme()->background = Style::Fill::SOLID;
          subcontrol->getTheme()->background.colorPrimary.set(ReyEngine::Colors::randColor());
          layout3->addChild(subcontrol);
      }
   }

   else if (args.getArg("--layoutTest")){
      Application::printDebug() << "Layout test!" << endl;
      auto mainVLayout = make_shared<VLayout>("MainVLayout");
      mainVLayout->setAnchoring(BaseWidget::Anchor::FILL);
      root->addChild(mainVLayout);
      bool flipFlop = false;
      std::shared_ptr<Layout> prevlayout = mainVLayout;
      for (int i=0;i<50;i++){
         std::shared_ptr<Layout> newlayout;
         if (flipFlop){
             newlayout = make_shared<VLayout>("Vlayout" + to_string(i));
         } else {
             newlayout = make_shared<HLayout>("Hlayout" + to_string(i));
         }
         flipFlop = !flipFlop;
         auto control = make_shared<Control>("control" + to_string(i));
         control->getTheme()->background.set(Style::Fill::SOLID);
         control->getTheme()->background.colorPrimary.set(ColorRGBA::random(254));
         newlayout->addChild(control);
         prevlayout->addChild(newlayout);
         prevlayout = newlayout;
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

   else if (args.getArg("--tileMapTest")){
      auto tilemap = make_shared<TileMap>("TileMap"); //either pass in a rect or do fit texture later
      FileSystem::File file = "test/spritesheet.png";
      auto layerOpt = tilemap->addLayer(file);
      if (layerOpt) {
         Application::printDebug() << "Tilemap added layer " << layerOpt.value() << " using sprite sheet " << tilemap->getLayer(layerOpt.value()).getAtlas().filePath.abs() << endl;
      } else {
         Application::printError() << "Tilemap " << file.abs() << " not found" << endl;
         return 1;
      }

      auto clickLayer = make_shared<Control>("ClickLayer");
      root->addChild(tilemap);
      auto knownShape = make_shared<Control>("knownShape");
      auto drawer = make_shared<Control>("drawer");
      root->addChild(drawer);
      auto knownShapeLabel = make_shared<Label>("label");
      knownShape->addChild(knownShapeLabel);
      knownShape->getTheme()->background.colorPrimary.value.a = 127;
      root->addChild(knownShape);
      tilemap->addChild(clickLayer);
      tilemap->setRect({100,100, 500,500});
      clickLayer->getTheme()->background.value = Style::Fill::NONE;
      clickLayer->setAnchoring(BaseWidget::Anchor::FILL);
//      tilemap->setAnchoring(BaseWidget::Anchor::FILL);
      clickLayer->setBackRender(true);

      auto render = [&](){
//         cout << "rect = " << clickLayer->getRect() << endl;
//         cout << "globalrect = " << clickLayer->getGlobalRect() << endl;
         drawRectangle(clickLayer->getRect(), Colors::red);
      };
      clickLayer->setRenderCallback(render);

      auto drawRender = [&](){
         DrawLine(0,0,100,100, BLUE);
      };
      drawer->setRenderCallback(drawRender);

      auto unhandledInput = [tilemap, clickLayer](const InputEvent& event, const std::optional<UnhandledMouseInput>& mouse) -> Handled {
         auto coords = mouse ? tilemap->getCoord(mouse.value().localPos) : TileMap::TileCoord(-1,-1);
         switch (event.eventId){
            case InputEventMouseButton::getUniqueEventId():{
               const auto& mbEvent = event.toEventType<InputEventMouseButton>();
               if (mbEvent.isDown) return false; //only uppies
               auto indexOpt = tilemap->getLayer(0).getTileIndex(coords);
               auto index = indexOpt ? indexOpt.value()+1 : 0;
               tilemap->getLayer(0).setTileIndex(coords, index);
               return true;}
            case InputEventMouseMotion::getUniqueEventId():
               const auto& mmEvent = event.toEventType<InputEventMouseMotion>();
               Application::printDebug() << "Mouse pos " << mouse.value().localPos << " = " << coords << endl;
               break;
         }
         return false;
      };
      clickLayer->setUnhandledInputCallback(unhandledInput);

      auto knownShapeRectCallback = [knownShape, knownShapeLabel](){
         knownShapeLabel->setText(knownShape->getGlobalPos());
      };
      knownShape->setRectChangedCallback(knownShapeRectCallback);
      knownShape->setRect(tilemap->getRect());

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
      auto vLayout = make_shared<VLayout>("VLayout");
      vLayout->setAnchoring(BaseWidget::Anchor::FILL);
      
      //create the 4 Controls
      auto controlTop = make_shared<Control>("TopControl");
      auto controlMiddle = make_shared<Control>("MiddleControl");
      auto controlBottom = make_shared<Control>("BottomControl");
      auto controlFill = make_shared<Control>("FillControl");

      vLayout->addChild(controlTop);
      vLayout->addChild(controlMiddle);
      vLayout->addChild(controlBottom);
      vLayout->addChild(controlFill);

      for (auto& control : {controlTop, controlMiddle, controlMiddle, controlFill}) {
         control->getTheme()->background.set(Style::Fill::SOLID);
         control->getTheme()->background.colorPrimary.set(Colors::randColor());
      }

      //create test labels for all anchoring
      auto noneLabel = make_shared<Label>("NoneLabel");
      noneLabel->setText("None");
      noneLabel->setAnchoring(BaseWidget::Anchor::NONE);
      controlTop->addChild(noneLabel);

      auto leftLabel = make_shared<Label>("LeftLabel");
      leftLabel->setText("Left");
      leftLabel->setAnchoring(BaseWidget::Anchor::LEFT);
      controlTop->addChild(leftLabel);

      auto rightLabel = make_shared<Label>("RightLabel");
      rightLabel->setText("Right");
      rightLabel->setAnchoring(BaseWidget::Anchor::RIGHT);
      controlTop->addChild(rightLabel);

      auto topLabel = make_shared<Label>("TopLabel");
      topLabel->setText("Top");
      topLabel->setAnchoring(BaseWidget::Anchor::TOP);
      controlTop->addChild(topLabel);

      auto bottomLabel = make_shared<Label>("BottomLabel");
      bottomLabel->setText("Bottom");
      bottomLabel->setAnchoring(BaseWidget::Anchor::BOTTOM);
      controlMiddle->addChild(bottomLabel);

      auto bottomLeftLabel = make_shared<Label>("BottomLeftLabel");
      bottomLeftLabel->setText("Bottom Left");
      bottomLeftLabel->setAnchoring(BaseWidget::Anchor::BOTTOM_LEFT);
      controlMiddle->addChild(bottomLeftLabel);

      auto bottomRightLabel = make_shared<Label>("BottomRightLabel");
      bottomRightLabel->setText("Bottom Right");
      bottomRightLabel->setAnchoring(BaseWidget::Anchor::BOTTOM_RIGHT);
      controlMiddle->addChild(bottomRightLabel);

      auto topLeftLabel = make_shared<Label>("TopLeftLabel");
      topLeftLabel->setText("Top Left");
      topLeftLabel->setAnchoring(BaseWidget::Anchor::TOP_LEFT);
      controlMiddle->addChild(topLeftLabel);

      auto topRightLabel = make_shared<Label>("TopRightLabel");
      topRightLabel->setText("Top Right");
      topRightLabel->setAnchoring(BaseWidget::Anchor::TOP_RIGHT);
      controlBottom->addChild(topLabel);

      auto fillButton = make_shared<PushButton>("FillLabel");
      fillButton->setText("Fill");
      fillButton->setAnchoring(BaseWidget::Anchor::FILL);
      controlFill->addChild(fillButton);

      auto centerLabel = make_shared<Label>("CenterLabel");
      centerLabel->setText("Center");
      centerLabel->setAnchoring(BaseWidget::Anchor::CENTER);
      controlBottom->addChild(centerLabel);

      for (auto& label : {noneLabel, leftLabel, rightLabel, topLabel, bottomLabel, bottomLeftLabel, bottomRightLabel, topLeftLabel, topRightLabel, centerLabel}) {
         label->getTheme().get()->background.set(Style::Fill::SOLID);
         label->getTheme().get()->background.colorPrimary.set(Colors::green);
      }
      
      root->addChild(vLayout);
   }

   else if (args.getArg("--marginsTest")){
       //make a grid of layouts
       auto mainvlayout = make_shared<VLayout>("MainVLayout");
       mainvlayout->setAnchoring(BaseWidget::Anchor::FILL);
       root->addChild(mainvlayout);
       static constexpr int ROWS = 10;
       static constexpr int COLUMNS = 10;
       mainvlayout->getTheme()->layoutMargins.setAll(5);
       for (int i=0; i<ROWS; i++){
           auto row = make_shared<HLayout>("Row" + to_string(i));
           row->getTheme()->layoutMargins.setAll(5);
           mainvlayout->addChild(row);
           //add column to row
           for (int j=0; j<COLUMNS; j++){
               auto column = make_shared<VLayout>("Row" + to_string(i) + "Column" + to_string(j));
               row->addChild(column);
               //set the margins
               column->getTheme()->layoutMargins.setAll(i+j);
               //add a control
               auto control = make_shared<Control>("Control " + to_string(i+j));
               column->addChild(control);
               //give it a color
               control->getTheme()->background = Style::Fill::SOLID;
               control->getTheme()->background.colorPrimary = Colors::randColor();

              //add a label
              auto label = make_shared<Label>("Label");
              auto onEnter = [label, control](){
                  label->setVisible(true);
                  label->setText(control->localToGlobal(label->getPos()));
               };
              auto onExit = [label, control](){
                 label->setVisible(false);
              };
              control->setMouseEnterCallback(onEnter);
              control->setMouseExitCallback(onExit);
              control->setAcceptsHover(true); //get mouse enter and mouse exit events
              label->setPos(2,2);
              label->setVisible(false);
              control->addChild(label);
           }
       }
   }

   else if (args.getArg("--hoverTest")){
      auto mainVLayout = make_shared<VLayout>("VLayout");
      mainVLayout->setAnchoring(BaseWidget::Anchor::FILL);
      root->addChild(mainVLayout);
      for (int i=0; i<5; i++){
         auto control = make_shared<Control>("Control" + to_string(i));
         auto onHover = [control](){
            std::cout << control->getName() << " got hover!" << endl;
            control->setVisible(true);
         };
         auto offHover = [control](){control->setVisible(false);std::cout << control->getName() << " got hover!" << endl;};
         control->setMouseEnterCallback(onHover);
         control->setMouseExitCallback(offHover);
         control->setVisible(false);
         control->getTheme()->background = Style::Fill::SOLID;
         control->getTheme()->background.colorPrimary = ReyEngine::ColorRGBA::random();
         mainVLayout->addChild(control);
      }
   }

   else if (args.getArg("--tabContainerTest")){
      Application::printDebug() << "Starting tab container test!" << endl;
      auto mainVLayout = make_shared<VLayout>("MainVLayout");
      auto tabContainer = make_shared<TabContainer>("TabContainer");
      mainVLayout->setAnchoring(BaseWidget::Anchor::FILL);
      mainVLayout->addChild(tabContainer);
      mainVLayout->getTheme()->layoutMargins.setAll(5);
      root->addChild(mainVLayout);

      //create some widgets to dislpay in the tabcontainer
      static constexpr int TAB_COUNT = 4;
      for (int i=0; i<TAB_COUNT; i++){
         auto control = make_shared<Control>("Control" + to_string(i));
         auto color = Colors::randColor();
         auto renderCB = [control, color](){
            ReyEngine::drawRectangle(control->getGlobalRect(), color);
         };
         control->setRenderCallback(renderCB);
         tabContainer->addChild(control);
      }

   }

   else if (args.getArg("--xmlTest")){
      //parse xml file
      FileSystem::File xmlFile("test/xmltest.xml");
      auto vlayout = make_shared<VLayout>("mainvlayout");
      vlayout->setAnchoring(BaseWidget::Anchor::FILL);
      root->addChild(vlayout);

      auto document = XML::Parser::loadFile(xmlFile);
      auto xmlRoot = document->getRoot();
      auto rootLabel = make_shared<Label>("rootLabel");
      rootLabel->setText(xmlRoot->name() + " =" + xmlRoot->value());
      rootLabel->setMinSize({999, 30});
      rootLabel->setMaxSize({999, 30});
      vlayout->addChild(rootLabel);

      int i=0;
      int indentLevel=0;
      std::function<void(const std::shared_ptr<XML::Element>&)> catElement = [&](const std::shared_ptr<XML::Element>& element){
         auto label = make_shared<Label>(to_string(i++));
         std::string indent(4 * indentLevel, ' ');
         label->setText(indent + "<" + element->name());
         label->setMinSize({999, 30});
         label->setMaxSize({999, 30});
         vlayout->addChild(label);
         std::optional<std::shared_ptr<XML::Attribute>> optAttr = element->firstAttr();
         while (optAttr){
            auto attr = optAttr.value();
            auto name = attr->name();
            auto value = attr->value();
            label->appendText(" " + name + "=""" + value + """");
            optAttr = attr->next();
         }
         label->appendText(">");
         auto value = element->value();
         if (!value.empty()){
            label->appendText(value);
         }

         auto optChild = element->firstChild();
         std::shared_ptr<Label> retval;
         if (optChild){
            indentLevel++;
            catElement(optChild.value());
            indentLevel--;
         }

         if (optChild) {
            label->appendText("\n");
         }
         label->appendText("</" + element->name() + ">");

         auto sibling = element->nextSibling();
         if (sibling){
            catElement(sibling.value());
         }
         return label;
      };
      //rebuild tree
      catElement(xmlRoot);
   }

   else if (args.getArg("--relativeMotionTest")){
      //make something we can draw on
      auto canvasControl = make_shared<Control>("Control");
      root->addChild(canvasControl);
      canvasControl->setAnchoring(BaseWidget::Anchor::FILL);
      Rect<int> rect(0,0,50,50);
      ReyEngineFont font;
      bool down = false;
      Pos<int> mousePos;
      Pos<int> offset;

      auto cbDrawCanvas = [&](){
         ReyEngine::drawRectangle(canvasControl->getRect(), Colors::lightGray);
         ReyEngine::drawRectangle(rect, down ? Colors:: red : Colors::green);

         //rect delta line
         if (down) {
            ReyEngine::drawLine({mousePos, mousePos - offset}, 1, Colors::blue);
            ReyEngine::drawText("offset = " + offset.toString(), mousePos + Pos<int>(15, 15), font);
         }
      };

      auto cbInput = [&](const InputEvent& event, std::optional<UnhandledMouseInput> mouse) -> bool {
         switch (event.eventId) {
            case InputEventMouseButton::getUniqueEventId(): {
               auto &mbEvent = event.toEventType<InputEventMouseButton>();
               down = mbEvent.isDown && rect.isInside(mbEvent.globalPos);
               return true;
            }
            case InputEventMouseMotion::getUniqueEventId():
               mousePos = InputManager::getMousePos();
               if (down) {
                  rect.setPos(mousePos - offset);
               } else {
                  offset = mousePos - rect.pos();
               }

               return true;
         }
         return false;
      };

      canvasControl->setRenderCallback(cbDrawCanvas);
      canvasControl->setUnhandledInputCallback(cbInput);
   }

   else if (args.getArg("--comboBoxTest")){
      Size<int> maxSize = {200, 30};
      auto mainvlayout = make_shared<VLayout>("MainVLayout");
      mainvlayout->getTheme()->layoutMargins.setAll(2);
      mainvlayout->setAnchoring(BaseWidget::Anchor::FILL);
      root->addChild(mainvlayout);

      for (int i=0;i<20;i++){
         auto combobox = make_shared<ComboBox>("Combobox" + to_string(i));
         combobox->setMaxSize(maxSize);
         mainvlayout->addChild(combobox);

         //create a struct to hold data
         struct ColorData : public ComboBoxData{
            ColorData(const std::string& name, const ColorRGBA& color): name(name), color(color){}
            std::string name;
            ColorRGBA color;
         };

         //instantiate some data
         std::vector<std::shared_ptr<ColorData>> colors = {
            make_shared<ColorData>("red", Colors::red),
            make_shared<ColorData>("green", Colors::green),
            make_shared<ColorData>("blue", Colors::blue)
         };

         auto hoverCB = [&](const ComboBox::EventComboBoxItemHovered& event){
            auto combobox = event.publisher->toBaseWidget()->toType<ComboBox>();
            auto data = static_pointer_cast<ColorData>(event.field.data);
            combobox->getTheme()->background.colorPrimary = data->color;
         };

         auto selectCB = [&](const ComboBox::EventComboBoxItemSelected& event){
            auto combobox = event.publisher->toBaseWidget()->toType<ComboBox>();
            auto data = static_pointer_cast<ColorData>(event.field.data);
            combobox->getTheme()->background.colorPrimary = data->color;
         };

         auto menuOpenCB = [&](const ComboBox::EventComboBoxMenuOpened& event){
            /**/
         };

         auto menuCloseCB = [&](const ComboBox::EventComboBoxMenuClosed& event){
            auto combobox = event.publisher->toBaseWidget()->toType<ComboBox>();
            auto data = static_pointer_cast<ColorData>(combobox->getCurrentField().data);
            combobox->getTheme()->background.colorPrimary = data->color;
         };

         //pack the comboboxes with the data
         for (auto& color : colors) {
            combobox->addItem(color->name);
            combobox->getLastField().data = color;
            combobox->subscribe<ComboBox::EventComboBoxItemHovered>(combobox, hoverCB);
            combobox->subscribe<ComboBox::EventComboBoxMenuOpened>(combobox, menuOpenCB);
            combobox->subscribe<ComboBox::EventComboBoxMenuClosed>(combobox, menuCloseCB);
            combobox->subscribe<ComboBox::EventComboBoxItemSelected>(combobox, selectCB);
         }
      }
   }

   else if(args.getArg("--configTest")){
      Config::loadConfig("test/test.lua");
      auto vlayout = make_shared<VLayout>("Layout");
      vlayout->setAnchoring(BaseWidget::Anchor::FILL);
      root->addChild(vlayout);
      for (auto& label : {
            make_shared<Label>(Config::getString("testmsg")),
            make_shared<Label>(Config::getString("testtable", "value")),
            make_shared<Label>("an integer = " + to_string(Config::getInt("an_integer"))),
      })
      {
         label->setMaxSize({9999, 30});
         vlayout->addChild(label);
      }
   }

   else if(args.getArg("--lineEditTest")){
      static constexpr int ROW_HEIGHT = 30;
      Size<int> maxSize = {200, ROW_HEIGHT};
      auto vlayout = make_shared<VLayout>("Layout");
      vlayout->setAnchoring(BaseWidget::Anchor::FILL);
      vlayout->getTheme()->layoutMargins.setAll(2);
      root->addChild(vlayout);

      for (int i=0;i<20;i++){
         auto hlayout = make_shared<HLayout>("HLayout" + to_string(i));
         hlayout->getTheme()->layoutMargins.setAll(2);
         hlayout->setMaxSize({9999,ROW_HEIGHT});
         vlayout->addChild(hlayout);
         auto linedit = make_shared<LineEdit>("LineEdit" + to_string(i));
         linedit->setDefaultText("DefaultText");
         linedit->setMaxSize(maxSize);
         hlayout->addChild(linedit);

         //make a label to echo the contents of the linedit
         auto label = make_shared<Label>("Label" + to_string(i));
         label->clear();
         //create callback
         auto cbTextChanged = [label](const LineEdit::EventLineEditTextChanged& event){
            label->setText(event.newText);
         };
         label->subscribe<LineEdit::EventLineEditTextChanged>(linedit, cbTextChanged);
         hlayout->addChild(label);
      }
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

   root->setAnchoring(BaseWidget::Anchor::FILL);
   window->exec();
   return 0;
}