#include "Window.h"
#include "Canvas.h"
#include "ui/UI.h"

using namespace std;
using namespace ReyEngine;

int main() {
   //create window
   {
      auto& window = Application::createWindowPrototype("window", 800, 600, {WindowFlags::RESIZE}, 60)->createWindow();
      auto root = window.getCanvas();

      //create editor
      std::shared_ptr<TileEditor> editor;
      editor = make_child<TileEditor>(root->getNode(), "Editor");

      //create UI
      auto ui = make_child<UI>(root->getNode(), "UI", editor);
      ui->setAnchoring(Anchor::FILL);
      ui->moveToForeground();

      window.exec();
      return 0;
   }
}