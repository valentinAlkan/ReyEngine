#include "Window.h"
#include "Canvas.h"
#include "UI.h"

using namespace std;
using namespace ReyEngine;

int main() {
   //create window
   {
      auto& window = Application::createWindowPrototype("window", 1920, 1080, {WindowFlags::RESIZE}, 60)->createWindow();
      auto root = window.getCanvas();

      //create UI
      {
         auto [gameWorld, node] = make_node<UI>("UI");
         root->addChild(std::move(node));
         gameWorld->setAnchoring(Anchor::FILL);
      }

      window.exec();
      return 0;
   }
}