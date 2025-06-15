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
#include "Sprite.h"
#include "UDP.h"

using namespace std;
using namespace ReyEngine;
using namespace ReyEngine::Internal::Tree;
using namespace Sockets;

struct NetworkWidget : public Widget {
   REYENGINE_OBJECT(NetworkWidget)
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


int main() {
   //create window
   {
      auto& window = Application::createWindowPrototype("window", 1920, 1080, {WindowFlags::RESIZE}, 60)->createWindow();
      auto root = window.getCanvas();

      //create a udp sender to send messages from
      UDPSender sender("localhost", 8888);
      UDPListener listener;
      listener.listen("localhost", 8888);

      std::string msg = "helloworld!";
      sender.send(msg);

      //print out the message
      while (auto sock = listener.getNextReady(100ms)) {
         //print the message out
         char buf[128] = {0};
         auto bytesRead = sock->recv(buf, sizeof(buf));
         cout << "Got message : " << string(buf) << " from sender " << sock->getRecvAddr() << endl;
      }

      window.exec();
      return 0;
   }
}