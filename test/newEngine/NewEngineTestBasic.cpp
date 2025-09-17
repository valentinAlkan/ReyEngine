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

//   FileSystem::Directory folder = FileSystem::Directory(CrossPlatform::getUserLocalConfigDirApp());
//   if (folder.createIfNotExist()) Logger::info() << "Created folder " << folder.abs() << endl;
//   auto fileBrowser = make_child<FileBrowser>(root, "FileBrowser");
//   fileBrowser->setCurrentDirectory(folder);
//   fileBrowser->getTheme().background.colorPrimary = Colors::blue;
//   fileBrowser->setAnchoring(ReyEngine::Anchor::CENTER);
//   fileBrowser->setSize(800,600);

   auto scrollArea = make_child<ScrollArea>(root, "scrollArea");
   scrollArea->setRect(150,150,300,300);
   {
      auto label = make_child<Label>(scrollArea, "label", "this here is some text my friend");
      label->setPosition(123, 201);
   }
   {
      auto label = make_child<Label>(scrollArea, "label2", "this here is some text my friend part 2");
      label->setPosition(0, 100);
   }

   auto control = make_child<Control>(root, "control");
//   control->setRect(0,0,scrollArea->getPos().x * 2,scrollArea->getPos().y * 2);
//   cout << "scroll area should be located at " << scrollArea->getPos() << " but is actually located at " << control->getRect().bottomRight() << endl;
//   control->setRect(100,100,100,100);

   auto processCallback = [&](Control& ctl, float dt){
      auto pos = InputManager::getMousePos();
      Logger::info() << pos << endl;
//      scrollArea->setPosition(pos.get());
   };

   auto renderCallback = [&](const Control& ctl){
      drawRectangleLines(scrollArea->getRect(), 1.0, Colors::blue);
   };

   control->setProcessCallback(processCallback, true);
   control->setRenderCallback(renderCallback);

   window.exec();
   return 0;
}