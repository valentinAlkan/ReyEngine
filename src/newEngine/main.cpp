#include "Window.h"
#include "Widget.h"
#include "Canvas.h"
#include "Label.h"
#include "Layout.h"
#include "Button.h"
#include "Slider.h"
#include "LineEdit.h"
#include "TabContainer.h"
#include "ComboBox.h"
#include "ScrollArea.h"

using namespace std;
using namespace ReyEngine;
using namespace ReyEngine::Internal::Tree;

struct TestWidget : public Widget {
  REYENGINE_OBJECT(TestWidget)
  TestWidget(const std::string& text)
  : text(text)
  , color(Colors::randColor())
  {
     setSize({200,200});
  }
   void render2D() const override {
      drawRectangle(getRect().toSizeRect(), isInside ? color : Colors::lightGray);
      drawText(text, {0,0}, getDefaultFont());
      drawText("P = " + Pos<int>(getPos()).toString(), {0, 20}, getDefaultFont());
      drawText("S = " + Size<int>(getSize()).toString(), {0,40}, getDefaultFont());
      drawText(localMousePos, {0,60}, getDefaultFont());
   }
protected:
   void _on_rect_changed() override {
     cout << "new position = " << getPos() << endl;
  }
   Widget* _unhandled_input(const InputEvent& event) override {
     if (auto isMouse = event.isMouse()){
        localMousePos = isMouse.value()->getLocalPos();
        isInside = isMouse.value()->isInside();
        switch (event.eventId) {
           case InputEventMouseButton::ID:{
              auto& mbEvent = event.toEvent<InputEventMouseButton>();
              if (isInside && mbEvent.isDown) {
                 isDown = true;
                 return this;
              }
              if (isDown && !mbEvent.isDown){
                 isDown = false;
                 return this;
              }
              break;}
           case InputEventMouseMotion::ID:{
              auto& mmEvent = event.toEvent<InputEventMouseMotion>();
              if (isDown){
                 cout << "----------------------" << endl;
                 cout << getName() << endl;
                 cout << "Mouse delta = " << mmEvent.mouseDelta << endl;
                 cout << "current position = " << getPos() << endl;
                 cout << "new position = " << mmEvent.mouseDelta + getPos() << endl;
                 setPosition(getPos() + mmEvent.mouseDelta);
                 return this;
              }
              break;}
        }
     }

     return nullptr;
  }
  Pos<int> localMousePos;
  bool isInside = false;
  std::string text;
  ColorRGBA color;
  bool isDown = false;
};

int main(){
   //create window
   {
      auto& window = Application::createWindowPrototype("window", 1920, 1080, {WindowFlags::RESIZE}, 60)->createWindow();
      auto root = window.getCanvas();

//      std::vector<TypeNode*> labels;
//      {
//         auto [widget2, _] = make_node<TestWidget>("LabelParent", "Parent");
//         labels.push_back(root->getNode()->addChild(std::move(_)));
//         widget2->setPosition({0, 0});
//      }
//      {
//         auto [widget3, _] = make_node<TestWidget>("LabelChild", "child");
//         labels.push_back(labels.at(0)->addChild(std::move(_)));
//         widget3->setPosition({150, 150});
//      }
//
//      {
//         auto [label, _] = make_node<Label>("LabelHeyBabe", "Hey baby");
//         labels.push_back(labels.at(0)->addChild(std::move(_)));
//         label->setPosition({100, 500});
//      }

      //create a layout
      {
         TypeNode* layoutl;
         TypeNode* layoutr;
         {
            auto [layout, node] = make_node<Layout>("Layout", Layout::LayoutDir::VERTICAL);
            root->getNode()->addChild(std::move(node));
            layout->setAnchoring(Anchor::FILL);

            auto [_layoutl, nodel] = make_node<Layout>("Layoutl", Layout::LayoutDir::HORIZONTAL);
            layoutl = layout->getNode()->addChild(std::move(nodel));

            auto [_layoutr, noder] = make_node<Layout>("Layoutr", Layout::LayoutDir::HORIZONTAL);
            layoutr = layout->getNode()->addChild(std::move(noder));
         }
         TypeNode* widgetsHolder;
         TypeNode* buttonHolder;
         TypeNode* tabHolder;
         TypeNode* subCanvasHolder;
         TypeNode* scrollAreaHolder;
         // add some children to the layout
         {
            auto [widget1, n1] = make_node<TestWidget>("TestWidget1", "firstchild");
            auto [widget2, n2] = make_node<Layout>("ScrollArea Holder", Layout::LayoutDir::VERTICAL);
            auto [widget3, n3] = make_node<Layout>("SubCanvasHolder", Layout::LayoutDir::VERTICAL);
            auto [widget4, n4] = make_node<Layout>("TabLayout", Layout::LayoutDir::VERTICAL);
            auto [widget5, n5] = make_node<Layout>("WidgetsLayout", Layout::LayoutDir::VERTICAL);
            auto [widget6, n6] = make_node<Layout>("ButtonLayout", Layout::LayoutDir::VERTICAL);
            layoutl->addChild(std::move(n1));
            scrollAreaHolder = layoutl->addChild(std::move(n2));
            subCanvasHolder = layoutl->addChild(std::move(n3));
            tabHolder = layoutr->addChild(std::move(n4));
            widgetsHolder = layoutr->addChild(std::move(n5));
            buttonHolder = layoutr->addChild(std::move(n6));
         }
         //add some other widgets
         {
            auto [slider1, n1] = make_node<Slider>("slider1", Slider::SliderType::HORIZONTAL);
            widgetsHolder->addChild(std::move(n1));
            auto [lineedit1, n2] = make_node<LineEdit>("LineEdit", "Try clicking on this line edit");
            widgetsHolder->addChild(std::move(n2));
            auto [combobox1, n3] = make_node<ComboBox>("ComboBox");
            combobox1->addItems({"this", "are", "some", "items"});
            widgetsHolder->addChild(std::move(n3));
         }


         //add some buttons
         {
            auto [button1, n1] = make_node<PushButton>("button1");
            auto [button2, n2] = make_node<PushButton>("button2");
            auto [button3, n3] = make_node<PushButton>("button3");
            auto [button4, n4] = make_node<PushButton>("button4");
            buttonHolder->addChild(std::move(n1));
            buttonHolder->addChild(std::move(n2));
            buttonHolder->addChild(std::move(n3));
            buttonHolder->addChild(std::move(n4));
         }

         //make a tab widget
         {
            TypeNode* tabContainer;
            {
               auto [container, n] = make_node<TabContainer>("tabContainer");
               tabContainer = tabHolder->addChild(std::move(n));
            }

            //make takes
            {
               //add some stuff ot differentiate the pages
               auto [label1, n1] = make_node<Label>("Label1", "Page1");
               auto [label2, n2] = make_node<Label>("Label2", "Page2");
               auto [label3, n3] = make_node<Label>("Label3", "Page3");
               auto [label4, n4] = make_node<Label>("Label4", "Page4");
               tabContainer->addChild(std::move(n1));
               tabContainer->addChild(std::move(n2));
               tabContainer->addChild(std::move(n3));
               tabContainer->addChild(std::move(n4));
            }
         }

         // add a subcanvas
         {
            auto [subcanvas, n1] = make_node<Canvas>("SubCanvas");
            auto subcanvasNode = subCanvasHolder->addChild(std::move(n1));
            subcanvas->setAnchoring(ReyEngine::Anchor::FILL);

            //add a label to the subcanvas
            auto [label, n2] = make_node<TestWidget>("SubCanvasTestWidget", "ah fuck");
            subcanvasNode->addChild(std::move(n2));
            label->setPosition({100,100});
         }

         // add a scroll area
         {
            auto [scrollArea, n1] = make_node<ScrollArea>("ScrollArea");
            auto scrollAreaNode = scrollAreaHolder->addChild(std::move(n1));

            //add a label to the scrollArea
            auto [testWidget, n2] = make_node<TestWidget>("ScrollAreaTestWidget", "scrollTest");
            scrollAreaNode->addChild(std::move(n2));
            testWidget->setPosition({100, 100});

            //create a mouse event and test it
            InputEventMouseButton eventMouseButton(&window, scrollArea->getGloablRect().get().pos(), InputInterface::MouseButton::LEFT, true);
            window.processInput(eventMouseButton);
         }

      }

//      //scroll area
//      auto [scrollArea, node] = make_node<ScrollArea>("ScrollArea");
//      TypeNode* root->getNode()->addChild(std::move(node));
//      scrollArea->setRect(100, 200, 500, 500);

      window.exec();
   }
   return 0;

}