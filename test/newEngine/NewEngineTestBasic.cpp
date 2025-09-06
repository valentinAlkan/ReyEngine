#include "ReyEngine.h"
#include "Logger.h"
#include "Application.h"
#include "Window.h"
#include "Widget.h"
#include "Panel.h"
#include "Button.h"

using namespace ReyEngine;
using namespace std;

int main(){
   auto& window = Application::createWindowPrototype("window", 1920, 1080, {WindowFlags::RESIZE}, 60)->createWindow();
   auto root = window.getCanvas();

   auto panel = make_child<Panel>(root->getNode(), "Panel");
   panel->setSize(300,300);
   window.exec();
   return 0;
}