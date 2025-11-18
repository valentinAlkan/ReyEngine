#include "ReyEngine.h"
#include "Application.h"
#include "Window.h"
#include "Widget.h"
#include "FileBrowser.h"
#include "Label.h"

using namespace ReyEngine;
using namespace std;

int main(){
   auto& window = Application::createWindowPrototype("window", 800, 600, {WindowFlags::RESIZE}, 60)->createWindow();
   auto root = window.getCanvas();

#if 0
   FileSystem::Directory folder = FileSystem::Directory(CrossPlatform::getUserLocalConfigDirApp());
   if (folder.createIfNotExist()) Logger::info() << "Created folder " << folder.abs() << endl;
   auto fileBrowser = make_child<FileBrowser>(root, "FileBrowser");
   fileBrowser->setCurrentDirectory(folder);
   fileBrowser->getTheme().background.colorPrimary = Colors::blue;
   fileBrowser->setAnchoring(ReyEngine::Anchor::CENTER);
   fileBrowser->setSize(800,600);
   fileBrowser->setVisible(true);

   //send some input to 20,50
   InputEventMouseMotion e(root.get(), {20,50}, {0,0});
   Application::getWindow(0).processInput(e);
#else
   auto lineEdit = make_child<LineEdit>(root, "lineEdit");
   lineEdit->setAnchoring(ReyEngine::Anchor::TOP_RIGHT);
   lineEdit->setSize(200, 22);
   auto scrollArea1 = make_child<ScrollArea>(root, "rootscrollArea1");
   auto scrollArea2 = make_child<ScrollArea>(root, "rootscrollArea2");
   scrollArea2->setPosition(800, 800);
   scrollArea2->setSize(300, 300);
   //broadcast input events
   scrollArea1->setInputFiltering(ReyEngine::InputFilter::PASS_PROCESS_PUBLISH);
   scrollArea1->setPosition(100, 100);
   scrollArea1->setSize(300, 300);
   auto label1 = make_child<Label>(scrollArea1, "label1", "this here is some text my friend this part is super long for reasons");
   label1->setPosition(123, 201);
   label1->getTheme().font = std::make_shared<ReyEngineFont>("/root/casper/ReyEngine/assets/fonts/VollkornRegular.ttf");
   make_child<Label>(scrollArea1, "label2", "this here is someasdkljhalksjdf text my friend part 2")->setPosition(0, 100);
   make_child<PushButton>(scrollArea1, "btn", "im a computer")->setPosition(30, 300);

   auto control = make_child<Control>(root, "rootcontrol");
   auto processCallback = [&](Control& ctl, float dt){
//      auto pos = InputManager::getMousePos();
//      Logger::info() << pos << endl;
//      scrollArea->setPosition(pos.get());
   };

   Pos<float> inputPosScrollAreaLocal;

   auto inputCallback = [&](Widget::WidgetUnhandledInputEvent& e){
      if (auto event = e.fwdEvent.isEvent<const InputEventMouseMotion>()){
         auto localpos = event.value()->mouse.getLocalPos();
         Logger::info() << event.value()->mouse.getLocalPos() << endl;
         inputPosScrollAreaLocal = localpos;
         scrollArea1->getChild("label1").value()->as<Label>().value()->setText(localpos.toString());
//         e.handler = scrollArea.get();
      }
   };

   control->setProcessCallback(processCallback, true);

   scrollArea1->subscribeMutable<Widget::WidgetUnhandledInputEvent&>(scrollArea1, inputCallback);

   auto scrollAreaRectChange = [&](const Widget::RectChangedEvent& e){
      lineEdit->setText(scrollArea1->getPos());
   };
   lineEdit->subscribe<Widget::RectChangedEvent>(scrollArea1, scrollAreaRectChange);

   auto lineEditTextChange = [&](const LineEdit::EventLineEditTextEntered& e){
      if (auto validPos = Pos<float>::fromString(lineEdit->getText())){
         auto pos = validPos.value();
         scrollArea1->setPosition(pos);
      } else {
         lineEdit->setText(scrollArea1->getPos());
      }
   };
   lineEdit->subscribe<LineEdit::EventLineEditTextEntered>(lineEdit, lineEditTextChange);

   while(1) {
      replace_child<ScrollArea>(root, "rootScrollArea1");
   }

#endif

   window.exec();
   return 0;
}