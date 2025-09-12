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

   FileSystem::Directory folder = FileSystem::Directory(CrossPlatform::getUserLocalConfigDir()) + "myApp";
   if (folder.createIfNotExist()) Logger::info() << "Created folder " << folder.abs() << endl;
   auto fileBrowser = make_child<FileBrowser>(root->getNode(), "FileBrowser");
   fileBrowser->setCurrentDirectory(folder);
   fileBrowser->getTheme().background.colorPrimary = Colors::blue;
   fileBrowser->setAnchoring(ReyEngine::Anchor::CENTER);
   fileBrowser->setSize(800,600);

//   auto tree = make_child<Tree>(root->getNode(), "tree");
//   tree->setRoot(make_unique<TreeItem>("Root"));
//   tree->getRoot().value()->push_back(make_unique<TreeItem>("Item1"));
//   tree->setAnchoring(ReyEngine::Anchor::CENTER);
//   tree->setMinSize(500,500);

   window.exec();
   return 0;
}