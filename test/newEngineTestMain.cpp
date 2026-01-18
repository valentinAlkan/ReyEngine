#include "Window.h"
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
#include "Sprite.h"
#include "Panel.h"
#include "Tree.h"
#include "Panel.h"
#include "Group.h"

using namespace std;
using namespace ReyEngine;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct PosTestWidget : public Widget {
   REYENGINE_OBJECT(PosTestWidget)

   PosTestWidget(const std::string& text)
   : text(text), color(Colors::randColor()) {
      setSize({200, 200});
   }

   void render2D() const override {
      float ypos = 0;
      bool _isInsideAtRenderTime = getLocalMousePos().isInside(getSizeRect());
      drawRectangle(getSizeRect(), _isInsideAtRenderTime ? color : Colors::lightGray);
      drawText(text, {0, ypos+=20}, theme->font);
      drawText("Pos = " + Pos<int>(getPos()).toString(), {0, ypos+=20}, theme->font);
      drawText("Siz = " + Size<int>(getSize()).toString(), {0, ypos+=20}, theme->font);
      drawText("G" + globalMousePos.toString(), {0, ypos+=20}, theme->font);
      drawText("L" + localMousePos.toString(), {0, ypos+=20}, theme->font);
      drawText(to_string(Application::getWindow(0).getFrameCount()), {0, ypos+=20}, theme->font);
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
               static size_t inputNo = 0;
               auto& mmEvent = event.toEvent<InputEventMouseMotion>();
               if (isDown) {
                  cout << "----------------------" << endl;
                  cout << "no. : " << inputNo++ << endl;
                  cout << getName() << endl;
                  cout << "Mouse delta = " << mmEvent.mouseDelta << endl;
                  cout << "current position = " << getPos() << endl;
                  cout << "new position = " << mmEvent.mouseDelta + getPos() << endl;
                  setPosition(getPos() + mmEvent.mouseDelta.toPos());
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct SliderReactWidget : public Widget {
   REYENGINE_OBJECT(SliderReactWidget)
   SliderReactWidget() {setMaxSize(ReyEngine::Size<R_FLOAT>::Max().x, 30);}
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct ScissorWidget : public Widget {
   REYENGINE_OBJECT(ScissorWidget)
   ScissorWidget() {
      scissorRect = getSizeRect();
   }
   void render2D() const override {
      ScopeScissor scissor(scissorRect);
      drawRectangleGradientV(getSizeRect(), Colors::orange, Colors::purple);
   }
   void setScissorArea(const Rect<float>& r){
      scissorRect = r;
   }
   Rect<float> scissorRect;
};


int main() {
   //create window
   {
      auto& window = Application::createWindowPrototype("window", 1920, 1080, {WindowFlags::RESIZE}, 60)->createWindow();
      auto root = window.getCanvas();

      //create a layout
      {
         auto mainLayout = make_child<Layout>(root, "mainLayout", Layout::LayoutDir::VERTICAL);
         mainLayout->setAnchoring(Anchor::FILL);
         auto layoutl = make_child<Layout>(mainLayout, "Layoutl", Layout::LayoutDir::HORIZONTAL);
         auto layoutr = make_child<Layout>(mainLayout, "Layoutr", Layout::LayoutDir::HORIZONTAL);

         // add some children to the layout
         auto subcanvas = make_child<Canvas>(layoutl, "SubCanvas");
         auto ptw = make_child<PosTestWidget>(subcanvas, "SubCanvasTestWidget", "test");
         ptw->setPosition({100, 100});
         ptw->setFocused(true);
         auto panela = make_child<Panel>(subcanvas, "SubCanvasPanel");
         panela->setPosition({300, 100});
         panela->setSize(100,100);
         auto scrollArea = make_child<ScrollArea>(layoutl, "MainScrollArea");
         auto tabHolder = make_child<Layout>(layoutr, "TabLayout", Layout::LayoutDir::VERTICAL);
         auto widgetsHolder = make_child<Layout>(layoutr, "WidgetsLayout", Layout::LayoutDir::VERTICAL);
         auto buttonHolder = make_child<Group>(layoutr, "ButtonLayout");
         //add some other widgets
         {
            auto lineedit = make_child<LineEdit>(widgetsHolder, "LineEdit", "Try clicking on this line edit");
            auto combobox = make_child<ComboBox<int>>(widgetsHolder, "ComboBox");
            combobox->addItems({"this", "are", "some", "items"});

            auto lineEditCB = [&](const LineEdit::EventLineEditTextChanged& event) {
               cout << "New Line Edit Text Old = " << event.oldText << endl;
               cout << "New Line Edit Text New = " << event.newText << endl;
            };
            lineedit->subscribe<LineEdit::EventLineEditTextChanged>(lineedit, lineEditCB);

            auto slider = make_child<Slider>(widgetsHolder, "slider1", Slider::SliderType::HORIZONTAL);
            auto sliderReact = make_child<SliderReactWidget>(widgetsHolder, "sliderReact1");
            auto tree = make_child<Tree>(widgetsHolder, "tree");
            auto item = tree->setRoot("root");
            item->push_back("item1");
            item->push_back("item2");
            item->push_back("item3");


            //add slider reaction callback
            auto srCallback = [sliderReact](const Slider::EventSliderValueChanged& event) {
               sliderReact->setValue(event.pct);
            };
            sliderReact->subscribe<Slider::EventSliderValueChanged>(slider, srCallback);

         }

         //add some buttons
         {
            //make a button group
            auto summonButton = make_child<PushButton>(buttonHolder, "btnPopup", "Summon Popup");
            auto dialogHButton = make_child<PushButton>(buttonHolder, "btnDialogH", "Summon Horizontal Dialog");
            auto dialogVButton = make_child<PushButton>(buttonHolder, "btnDialogV", "Summon Vertical Dialog");
            auto button4 = make_child<PushButton>(buttonHolder, "button4");
         }

         //add a popup dialog
//         auto hDialog =


         //make a tab widget
         {
            auto tabContainer = make_child<TabContainer>(tabHolder, "tabContainer");

            //make tabs
            {
               //add some stuff ot differentiate the pages
               {
                  auto textureTestLayout = make_child<Layout>(tabContainer, "TextureTestLayout", Layout::LayoutDir::VERTICAL);
                  std::vector<std::pair<std::string, TextureRect::FitType>> fitTypes = {
                        {"FitRect",   TextureRect::FitType::FIT_RECT},
                        {"FitWidth",  TextureRect::FitType::FIT_WIDTH},
                        {"FitHeight", TextureRect::FitType::FIT_HEIGHT},
                        {"None",      TextureRect::FitType::NONE}
                  };
                  auto textureTestComboBox = make_child<ComboBox<TextureRect::FitType>>(textureTestLayout, "TextureTestComboBox", fitTypes);
                  auto textureRect = make_child<TextureRect>(textureTestLayout, "TextureTest", FileSystem::File("test/spritesheet.png"), TextureRect::FitType::FIT_RECT);

                  auto fitMenuCB = [textureRect](const ComboBox<TextureRect::FitType>::EventComboBoxItemSelected& event) {
                     cout << event.field->text << " at index " << event.itemIndex << " = " << (int) event.field->data << endl;
                     textureRect->setFitType(event.field->data);
                  };
                  textureTestComboBox->subscribe<ComboBox<TextureRect::FitType>::EventComboBoxItemSelected>(
                        textureTestComboBox, fitMenuCB);
               }

               auto drawTest = make_child<DrawTestWidget>(tabContainer, "DrawTest");
               auto spritesTab = make_child<Control>(tabContainer, "SpritesTest");
               auto sprite = make_child<Sprite>(spritesTab, "Sprite", FileSystem::File("test/characters.png"), Rect<R_FLOAT>(3, 4, 16, 16));
               sprite->setRect(20,20,64,64);
               constexpr Rect<R_FLOAT> _r = {3, 4, 16, 16};
               std::vector<Rect<R_FLOAT>> regions = {
                  _r,
                  _r + Pos<R_FLOAT>(16, 0),
               };
               auto animatedSprite = make_child<AnimatedSprite>(spritesTab, "AnimatedSprite", FileSystem::File("test/characters.png"), regions);
               animatedSprite->setRect(100,20,64,64);
               {
                  auto zoomCanvas = make_child<Canvas>(tabContainer, "ZoomTest");
                  zoomCanvas->setInputFiltering(InputFilter::PUBLISH_ONLY);
                  // connect to unhandled input signal

                  auto cbUnhandledInput = [zoomCanvas](Widget::WidgetUnhandledInputEvent& event){
                     if (event.fwdEvent.isEvent<InputEventMouseWheel>()){
                        const auto& mwEvent = event.fwdEvent.toEvent<InputEventMouseWheel>().wheelMove;
                        zoomCanvas->getCamera().zoom += zoomCanvas->getCamera().zoom * mwEvent.y * .1;
                        cout << "zoom = " << zoomCanvas->getCamera().zoom << endl;
                        event.handler = zoomCanvas.get();
                     }
                  };

                  zoomCanvas->subscribeMutable<Widget::WidgetUnhandledInputEvent&>(zoomCanvas, cbUnhandledInput);

                  //add a texture rect to the canvas' background
                  auto texRect = make_child<TextureRect>(zoomCanvas, "ZoomTextureRect");
                  zoomCanvas->setAnchoring(ReyEngine::Anchor::FILL);
                  texRect->setTexture(FileSystem::File("test/spritesheet.png"));
                  texRect->setAnchoring(ReyEngine::Anchor::FILL);
                  texRect->setFitType(ReyEngine::TextureRect::FitType::NONE);
                  // add some ui to the canvas
                  auto label = make_child<Label>(zoomCanvas, "UILabel", "This should be on the foreground");
                  label->setPosition({5,50});
                  zoomCanvas->moveToForeground(label.get());
               }

               {
                  auto scissorTest = make_child<ScissorWidget>(tabContainer, "ScissorTest");
                  scissorTest->setScissorArea({20,20,50,50});
               }
               drawTest->setAnchoring(ReyEngine::Anchor::FILL);
            }
         }

         // add a scroll area
         {
            //add a layout to the scroll area
            make_child<Control>(scrollArea, "ScrollAreaControl")->setSize({1000, 1000});
            make_child<PosTestWidget>(scrollArea, "ScrollAreaTestWidget", "scrollTest")->setPosition({100, 100});
            //add a button to the scrollArea
            make_child<PushButton>(scrollArea, "ScrollAreaTestBtn", "Scroll Button")->setPosition(200, 350);
            //add a panel to the scroll area
            auto panel = make_child<Panel>(scrollArea, "Panel");
            panel->setRect(100, 300, 200, 200);
            auto panelLabel = make_child<Label>(panel, "label");

         }

         //create popup control
         {
            auto popupCtl = make_child<Control>(root, "Popup");

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

               auto btnOk = make_child<PushButton>(popupCtl, "btnOk", "ok");
               auto btnCancel = make_child<PushButton>(popupCtl, "btnCancel", "cancel");
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
         using TestDialog = Dialog<4, testEnum>;
         //add a callback
         auto dialogCB = [](const TestDialog::DialogCloseEvent& e){
            Logger::info() << "Dialog box " << e.publisher->as<TestDialog>().value()->getNode()->name << " selected option " << e.asString << " which corresponds to an int value of " << (int)e.value << endl;
         };
         //create dialog H control
//         {
//            auto dialog = make_child<TestDialog>(root, "dialogH", options, "Test Text");
//            dialogHCtl->setPosition(dialogHCtl->getRect().centerOnPoint(root->getRect().center()).pos());
//            dialogHCtl->setVisible(false);
//         }
//
//         //create dialog V control
//         {
//            auto dialog = make_child<TestDialog>(root, "dialogV", options, "Test Text", Layout::LayoutDir::VERTICAL);
//            dialogVCtl->setPosition(dialogVCtl->getRect().centerOnPoint(root->getRect().center()).pos());
//            dialogVCtl->setVisible(false);
//         }
//
//         dialogHCtl->subscribe<TestDialog::DialogCloseEvent>(dialogHCtl, dialogCB);
//         dialogVCtl->subscribe<TestDialog::DialogCloseEvent>(dialogVCtl, dialogCB);
      }
      window.exec();
      return 0;
   }
}