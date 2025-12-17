#include "Window.h"

using namespace ReyEngine;
using namespace std;

int main(){
   auto& window = Application::createWindowPrototype("window", 800, 600, {WindowFlags::RESIZE}, 60)->createWindow();
   auto root = window.getCanvas();

   {
      auto easein = make_unique<Easing>(Easings::easeInSine, 2s);
      root->addEasing(std::move(easein));
   }


   window.exec();
   return 0;
}