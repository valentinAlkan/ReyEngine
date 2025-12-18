#include "Window.h"
#include "Label.h"

using namespace ReyEngine;
using namespace std;

int main(){
   auto& window = Application::createWindowPrototype("window", 800, 600, {WindowFlags::RESIZE}, 60)->createWindow();
   auto root = window.getCanvas();

   auto bounceLabel = make_child<Label>(root, "bounceLabel");

   std::function<void(Fraction)> easeIn;
   std::function<void(Fraction)> easeOut;
   easeIn = [&](Fraction f){
      bounceLabel->setPosition((root->getWidth() * f.get() - bounceLabel->getWidth()), root->getCenter().y);
   };

   easeOut = [&](Fraction f){
      bounceLabel->setPosition((root->getWidth() - (root->getWidth() * f.get()) - bounceLabel->getWidth()), root->getCenter().y);
   };


   bool doEaseIn = true;
   std::function<void()> reEase = [&](){
      root->addEasing(Easings::easeOutBounce, 2s, doEaseIn ? easeOut : easeIn, reEase);
      doEaseIn = !doEaseIn;
   };

   reEase();

   window.exec();
   return 0;
}