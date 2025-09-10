#include "FileBrowser.h"


using namespace std;
using namespace ReyEngine;
using namespace FileSystem;

/////////////////////////////////////////////////////////////////////////////////////////
void FileBrowser::render2D() const {

}

/////////////////////////////////////////////////////////////////////////////////////////
void FileBrowser::_init() {
   setMinSize(300,300);
   _tree = make_child<Tree>(getNode(), "tree");
   _tree->setMinSize(getMinSize());
   _tree->setMinSize(200,200);
   _tree->setAnchoring(Anchor::CENTER);

}

/////////////////////////////////////////////////////////////////////////////////////////
void FileBrowser::refreshDirectoryContents() {
   for (const auto& entry : _dir.listContents()){
   }
}