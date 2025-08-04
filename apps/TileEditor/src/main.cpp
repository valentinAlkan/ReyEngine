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
      {
         auto [_editor, node] = make_node<TileEditor>("Editor");
         root->addChild(std::move(node));
         editor = _editor;
      }

      //create UI
      {
         auto [ui, node] = make_node<UI>("UI", editor);
         root->addChild(std::move(node));
         ui->setAnchoring(Anchor::FILL);
         root->moveToForeground(ui.get());
      }

      window.exec();
      return 0;
   }
}