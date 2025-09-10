#include "ReyEngine.h"
#include "Application.h"
#include "Window.h"
#include "Widget.h"
#include "FileBrowser.h"

using namespace ReyEngine;
using namespace std;

int main(){
   auto& window = Application::createWindowPrototype("window", 1920, 1080, {WindowFlags::RESIZE}, 60)->createWindow();
   auto root = window.getCanvas();

   auto fileBrowser = make_child<FileBrowser>(root->getNode(), "FileBrowser", "/tmp");
   fileBrowser->getTheme().background.colorPrimary = Colors::blue;
   fileBrowser->setAnchoring(ReyEngine::Anchor::FILL);

//   fileBrowser->setCurrentDirectory("/tmp");

   window.exec();
   return 0;
}