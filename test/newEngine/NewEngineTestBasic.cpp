#include "ReyEngine.h"
#include "Application.h"
#include "Window.h"
#include "Widget.h"
#include "FileBrowser.h"
#include "Label.h"

using namespace ReyEngine;
using namespace std;

int main(){
   auto& window = Application::createWindowPrototype("window", 1920, 1080, {WindowFlags::RESIZE}, 60)->createWindow();
   auto root = window.getCanvas();

#if 0
   FileSystem::Directory folder = FileSystem::Directory(CrossPlatform::getUserLocalConfigDirApp());
   if (folder.createIfNotExist()) Logger::info() << "Created folder " << folder.abs() << endl;
   auto fileBrowser = make_child<FileBrowser>(root, "FileBrowser");
   fileBrowser->setCurrentDirectory(folder);
   fileBrowser->getTheme().background.colorPrimary = Colors::blue;
   fileBrowser->setAnchoring(ReyEngine::Anchor::CENTER);
   fileBrowser->setSize(800,600);
#else
   auto scrollArea = make_child<ScrollArea>(root, "rootscrollArea");
   //broadcast input events
   scrollArea->setInputFiltering(ReyEngine::InputFilter::PASS_PROCESS_PUBLISH);
   scrollArea->setPosition(100,100);
   auto label = make_child<Label>(root, "rootlabel", "This is where the scroll area should be");
   label->setPosition(100,100);
   scrollArea->setSize(300,300);
   make_child<Label>(scrollArea, "label1", "this here is some text my friend this part is super long for reasons")->setPosition(123, 201);
   make_child<Label>(scrollArea, "label2", "this here is someasdkljhalksjdf text my friend part 2")->setPosition(0, 100);
   make_child<PushButton>(scrollArea, "btn", "im a computer")->setPosition(30, 300);

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
         scrollArea->getChild("label1").value()->as<Label>().value()->setText(localpos.toString());
//         e.handler = scrollArea.get();
      }
   };

   auto renderCallback = [&](const Control& ctl){
      drawRectangleLines(scrollArea->getRect(), 1.0, Colors::blue);

      //draw a point relative to where the scroll area says it is
      Circle c(inputPosScrollAreaLocal + scrollArea->getPos(), 5);
      drawCircle(c, Colors::blue);

      drawRectangleLines(scrollArea->getRect() + scrollArea->getPos(), 1.0, Colors::red);
   };

   control->setProcessCallback(processCallback, true);
   control->setRenderCallback(renderCallback);

   scrollArea->subscribeMutable<Widget::WidgetUnhandledInputEvent&>(scrollArea, inputCallback);

#endif

   window.exec();
   return 0;
}