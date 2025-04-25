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
#include "TextureRect.h"
#include "Dialog.h"

using namespace std;
using namespace ReyEngine;
using namespace ReyEngine::Internal::Tree;

struct DrawTestWidget : public Widget {
   REYENGINE_OBJECT(DrawTestWidget)
   void render2D() const override {
      //draw rectangles
      auto [splitRectHL, splitRectHR] = getSizeRect().splitH<true>();
      auto [splitRectVLL, splitRectVLR] = splitRectHL.splitV<true>();
      auto [splitRectVRL, splitRectVRR]  = splitRectHR.splitV<true>();
      for (const auto& r: {splitRectHL, splitRectHR, splitRectVLL, splitRectVLR, splitRectVRL, splitRectVRR}) {
         drawRectangleLines(r, 1.0, Colors::red);
      }

      //split some more
      {
         auto [_1, _2] = splitRectVLL.splitV<true>();
         drawRectangle(_1, Colors::red);
         drawRectangle(_2, Colors::black);
      }

      {
         auto [_1, _2, _3, _4] = splitRectVRR.splitH<true>(10, 20, 30);
         drawRectangle(_1, Colors::red);
         drawRectangle(_2, Colors::black);
         drawRectangle(_3, Colors::red);
         drawRectangle(_4, Colors::blue);
      }

      drawText("Splits Test", {0, 0}, theme->font);
   }
};


struct TestWidget : public Widget {
   REYENGINE_OBJECT(TestWidget)

   TestWidget(const std::string& text)
         : text(text), color(Colors::randColor()) {
      setSize({200, 200});
   }

   void render2D() const override {
      bool _isInsideAtRenderTime = getLocalMousePos().isInside(getSizeRect());
      drawRectangle(getSizeRect(), _isInsideAtRenderTime ? color : Colors::lightGray);
      drawText(text, {0, 0}, getDefaultFont());
      drawText("Pos = " + Pos<int>(getPos()).toString(), {0, 20}, getDefaultFont());
      drawText("Siz = " + Size<int>(getSize()).toString(), {0, 40}, getDefaultFont());
      drawText("G" + globalMousePos.toString(), {0, 60}, getDefaultFont());
      drawText("L" + localMousePos.toString(), {0, 80}, getDefaultFont());
   }

protected:
   void _on_rect_changed() override {}

   Widget* _unhandled_input(const InputEvent& event) override {
      if (auto isMouse = event.isMouse()) {
         isInside = isMouse.value()->isInside();
         switch (event.eventId) {
            case InputEventMouseButton::ID: {
               auto& mbEvent = event.toEvent<InputEventMouseButton>();
               if (isInside && mbEvent.isDown) {
                  isDown = true;
                  return this;
               }
               if (isDown && !mbEvent.isDown) {
                  isDown = false;
                  return this;
               }
               break;
            }
            case InputEventMouseMotion::ID: {
               globalMousePos = isMouse.value()->getCanvasPos().get();
               localMousePos = isMouse.value()->getLocalPos();
               auto& mmEvent = event.toEvent<InputEventMouseMotion>();
               if (isDown) {
//                 cout << "----------------------" << endl;
//                 cout << getName() << endl;
//                 cout << "Mouse delta = " << mmEvent.mouseDelta << endl;
//                 cout << "current position = " << getPos() << endl;
//                 cout << "new position = " << mmEvent.mouseDelta + getPos() << endl;
                  setPosition(getPos() + mmEvent.mouseDelta);
                  return this;
               }
               break;
            }
         }
      }

      return nullptr;
   }

   Pos<int> localMousePos;
   Pos<int> globalMousePos;
   bool isInside = false;
   std::string text;
   ColorRGBA color;
   bool isDown = false;
};

struct SliderReactWidget : public Widget {
   REYENGINE_OBJECT(SliderReactWidget)
   SliderReactWidget() {setMaxSize({1000, 30});}
   void render2D() const override {
      auto [rectL, rectR] = getSizeRect().splitH<true>(_pct);
      drawRectangleGradientH(rectL, Colors::blue, Colors::black);
   }
   void setValue(const Percent& pct) {
      Logger::info() << "Reaction pct = " << _pct << endl;
      _pct = pct;
   }
   Percent _pct;
};

int main() {
   //create window
   {
      auto& window = Application::createWindowPrototype("window", 1920, 1080, {WindowFlags::RESIZE},
                                                        60)->createWindow();
      auto root = window.getCanvas();

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
         TypeNode* widgetsHolder = nullptr;
         TypeNode* buttonHolder = nullptr;
         TypeNode* tabHolder = nullptr;
         TypeNode* subCanvasHolder = nullptr;
         TypeNode* scrollAreaHolder = nullptr;
         TypeNode* popupNode = nullptr;
         TypeNode* dialogHNode = nullptr;
         TypeNode* dialogVNode = nullptr;

         // add some children to the layout
         {
//            auto [widget1, n1] = make_node<TestWidget>("TestWidget1", "firstchild");
            auto [widget2, n2] = make_node<Layout>("ScrollArea Holder", Layout::LayoutDir::VERTICAL);
            auto [widget3, n3] = make_node<Layout>("SubCanvasHolder", Layout::LayoutDir::VERTICAL);
            auto [widget4, n4] = make_node<Layout>("TabLayout", Layout::LayoutDir::VERTICAL);
            auto [widget5, n5] = make_node<Layout>("WidgetsLayout", Layout::LayoutDir::VERTICAL);
            auto [widget6, n6] = make_node<Layout>("ButtonLayout", Layout::LayoutDir::VERTICAL);
//            layoutl->addChild(std::move(n1));
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
            auto [sliderReact, sr1] = make_node<SliderReactWidget>("sliderReact1");
            widgetsHolder->addChild(std::move(sr1));

            //add slider reaction callback
            auto srCallback = [sliderReact](const Slider::EventSliderValueChanged& event) {
               sliderReact->setValue(event.pct);
            };
            sliderReact->subscribe<Slider::EventSliderValueChanged>(slider1.get(), srCallback);

            auto [lineedit1, n2] = make_node<LineEdit>("LineEdit", "Try clicking on this line edit");
            widgetsHolder->addChild(std::move(n2));
            auto [combobox1, n3] = make_node<ComboBox<int>>("ComboBox");
            combobox1->addItems({"this", "are", "some", "items"});
            widgetsHolder->addChild(std::move(n3));

            auto lineEditCB = [&](const LineEdit::EventLineEditTextChanged& event) {
               cout << "New Line Edit Text Old = " << event.oldText << endl;
               cout << "New Line Edit Text New = " << event.newText << endl;
            };
            lineedit1->subscribe<LineEdit::EventLineEditTextChanged>(lineedit1.get(), lineEditCB);
         }

         //add some buttons
         {
            auto [summonButton, n1] = make_node<PushButton>("btnPopup", "Summon Popup");
            auto [dialogHButton, n2] = make_node<PushButton>("btnDialogH", "Summon Horizontal Dialog");
            auto [dialogVButton, n3] = make_node<PushButton>("btnDialogV", "Summon Vertical Dialog");
            auto [button4, n4] = make_node<PushButton>("button4");
            buttonHolder->addChild(std::move(n1));
            buttonHolder->addChild(std::move(n2));
            buttonHolder->addChild(std::move(n3));
            buttonHolder->addChild(std::move(n4));

            auto cbPopup = [&](const PushButton::ButtonPressEvent& event) {
               auto isWidget = popupNode->as<Widget>();
               if (isWidget) {
                  auto& popup = isWidget.value();
                  popup->setVisible(true);
                  popup->setModal(true);
               }
            };
            summonButton->subscribe<PushButton::ButtonPressEvent>(summonButton.get(), cbPopup);

            //create dialog test
            auto cbDialogH = [&](const PushButton::ButtonPressEvent& event) {
               auto isWidget = dialogHNode->as<Widget>();
               if (isWidget) {
                  auto& dialog = isWidget.value();
                  dialog->setVisible(true);
                  dialog->setModal(true);
               }
            };
            dialogHButton->subscribe<PushButton::ButtonPressEvent>(dialogHButton.get(), cbDialogH);

            auto cbDialogV = [&](const PushButton::ButtonPressEvent& event) {
               auto isWidget = dialogVNode->as<Widget>();
               if (isWidget) {
                  auto& dialog = isWidget.value();
                  dialog->setVisible(true);
                  dialog->setModal(true);
               }
            };
            dialogVButton->subscribe<PushButton::ButtonPressEvent>(dialogVButton.get(), cbDialogV);

         }

         //make a tab widget
         {
            TypeNode* tabContainer;
            {
               auto [container, n] = make_node<TabContainer>("tabContainer");
               tabContainer = tabHolder->addChild(std::move(n));
            }

            //make tabs
            {
               //add some stuff ot differentiate the pages
               std::unique_ptr<TypeNode> p2;
               {
                  auto [textureTestLayout, n1] = make_node<Layout>("TextureTestLayout", Layout::LayoutDir::VERTICAL);
                  std::vector<std::pair<std::string, TextureRect::FitType>> fitTypes = {
                        {"FitRect",   TextureRect::FitType::FIT_RECT},
                        {"FitWidth",  TextureRect::FitType::FIT_WIDTH},
                        {"FitHeight", TextureRect::FitType::FIT_HEIGHT},
                        {"None",      TextureRect::FitType::NONE}
                  };
                  auto [textureTestComboBox, n2] = make_node<ComboBox<TextureRect::FitType>>("TextureTestComboBox", fitTypes);
                  auto [textureRect, n3] = make_node<TextureRect>("TextureTest", "test/spritesheet.png", TextureRect::FitType::FIT_RECT);
                  n1->addChild(std::move(n2));
                  n1->addChild(std::move(n3));
                  p2 = std::move(n1);

                  auto fitMenuCB = [textureRect](
                        const ComboBox<TextureRect::FitType>::EventComboBoxItemSelected& event) {
                     cout << event.field->text << " at index " << event.itemIndex << " = " << (int) event.field->data
                          << endl;
                     textureRect->setFitType(event.field->data);
                  };
                  textureTestComboBox->subscribe<ComboBox<TextureRect::FitType>::EventComboBoxItemSelected>(
                        textureTestComboBox, fitMenuCB);
               }

               auto [drawTest, p1] = make_node<DrawTestWidget>("DrawTest");
               auto [label3, p3] = make_node<Label>("Label3", "Page3");
               auto [label4, p4] = make_node<Label>("Label4", "Page4");
               tabContainer->addChild(std::move(p1));
               tabContainer->addChild(std::move(p2));
               tabContainer->addChild(std::move(p3));
               tabContainer->addChild(std::move(p4));
               drawTest->setAnchoring(ReyEngine::Anchor::FILL);
            }
         }

         // add a subcanvas
         {
            auto [subcanvas, n1] = make_node<Canvas>("SubCanvas");
            auto subcanvasNode = subCanvasHolder->addChild(std::move(n1));
            subcanvas->setAnchoring(ReyEngine::Anchor::FILL);

            //add a label to the subcanvas
            auto [label, n2] = make_node<TestWidget>("SubCanvasTestWidget", "test");
            subcanvasNode->addChild(std::move(n2));
            label->setPosition({100, 100});
         }

         // add a scroll area
         {
            TypeNode* scrollAreaNode;
            auto [scrollArea, _n1] = make_node<ScrollArea>("ScrollArea");
            {
               scrollAreaNode = scrollAreaHolder->addChild(std::move(_n1));
            }

            {
               //add a layout to the scroll area
               auto [_scrollAreaBackground, nscrolllayout] = make_node<Control>("ScrollAreaControl");
               _scrollAreaBackground->setSize({1000, 1000});
               scrollAreaNode->addChild(std::move(nscrolllayout));
            }

            //add a widget to the scrollArea
            {
               auto [testWidget, n2] = make_node<TestWidget>("ScrollAreaTestWidget", "scrollTest");
               scrollAreaNode->addChild(std::move(n2));
               testWidget->setPosition({100, 100});
            }

            //add a button to the scrollArea
            {
               auto [scrollButton, n3] = make_node<PushButton>("ScrollAreaTestBtn", "Scroll Button");
               scrollAreaNode->addChild(std::move(n3));
               scrollButton->setPosition(200, 350);
            }

            //create a mouse event and test it
            InputEventMouseButton eventMouseButton(&window, scrollArea->getGloablRect().get().pos(),
                                                   InputInterface::MouseButton::LEFT, true);
            window.processInput(eventMouseButton);
         }

         //create popup control
         {
            Control* popupCtl = nullptr;
            {
               auto [_popup, n1] = make_node<Control>("Popup");
               popupNode = root->getNode()->addChild(std::move(n1));
               popupCtl = _popup.get();
            }
            popupCtl->setSize(300, 100);
            popupCtl->setPosition(popupCtl->getRect().centerOnPoint(root->getRect().center()).pos());
            auto cbRender = [](const Control& ctrl) {
               drawRectangleRounded(ctrl.getSizeRect(), .1, 5, Colors::orange);
               drawRectangleRoundedLines(ctrl.getSizeRect(), .1, 5, 2.0, Colors::black);
            };
            //put some buttons on the control

            {
               auto onBtn = [&](const PushButton::ButtonPressEvent& event) {
                  popupCtl->setVisible(false);
                  popupCtl->setModal(false);
                  if (auto isButton = event.publisher->as<PushButton>()) {
                     cout << isButton.value()->getText() << endl;
                  }
               };

               auto [btnOk, nOk] = make_node<PushButton>("btnOk", "ok");
               auto [btnCancel, nCancel] = make_node<PushButton>("btnCancel", "cancel");
               popupCtl->getNode()->addChild(std::move(nOk));
               popupCtl->getNode()->addChild(std::move(nCancel));
               auto [L, R] = popupCtl->getSizeRect().splitH<true>();
               btnOk->setRect(L.embiggen(-20));
               btnCancel->setRect(R.embiggen(-20));

               btnOk->subscribe<PushButton::ButtonPressEvent>(btnOk.get(), onBtn);
               btnOk->subscribe<PushButton::ButtonPressEvent>(btnCancel.get(), onBtn);
            }
            popupCtl->setRenderCallback(cbRender);
            popupCtl->setVisible(false);
         }

         enum class testEnum { test1, test2, test3, test4};
         using namespace std::literals; // For string_view literals
         std::array options = {
               std::pair{std::string("dialog1"), testEnum::test1},
               std::pair{std::string("dialog2"), testEnum::test2},
               std::pair{std::string("dialog3"), testEnum::test3},
               std::pair{std::string("dialog4"), testEnum::test4}
         };
         using TestDialog = Dialog<testEnum, 4>;
         TestDialog* dialogHCtl = nullptr;
         TestDialog* dialogVCtl = nullptr;
         //add a callback
         auto dialogCB = [](const TestDialog::DialogCloseEvent& e){
            Logger::info() << "Dialog box " << e.publisher->as<TestDialog>().value()->getNode()->name << " selected option " << e.option << " which corresponds to an int value of " << (int)e.value << endl;
         };
         //create dialog H control
         {
            {
               auto [_dialog, n1] = make_node<TestDialog>("dialogH", options, "Test Text");
               dialogHNode = root->getNode()->addChild(std::move(n1));
               dialogHCtl = _dialog.get();
            }
            dialogHCtl->setPosition(dialogHCtl->getRect().centerOnPoint(root->getRect().center()).pos());
            dialogHCtl->setVisible(false);
         }

         //create dialog V control
         {
            {
               auto [_dialog, n1] = make_node<TestDialog>("dialogV", options, "Test Text", Layout::LayoutDir::VERTICAL);
               dialogVNode = root->getNode()->addChild(std::move(n1));
               dialogVCtl = _dialog.get();
            }
            dialogVCtl->setPosition(dialogVCtl->getRect().centerOnPoint(root->getRect().center()).pos());
            dialogVCtl->setVisible(false);
         }

         dialogHCtl->subscribe<TestDialog::DialogCloseEvent>(dialogHCtl, dialogCB);
         dialogVCtl->subscribe<TestDialog::DialogCloseEvent>(dialogVCtl, dialogCB);
      }
      window.exec();
      return 0;
   }
}