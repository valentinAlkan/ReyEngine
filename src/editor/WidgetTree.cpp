#include "WidgetTree.h"
#include "Label.hpp"

using namespace std;
using namespace ReyEngine;

/////////////////////////////////////////////////////////////////////////////////////////
void WidgetTree::_init(){
   //populate with widgets
   auto& typeManager = Internal::TypeManager::instance();
   auto typeMetaRoot = typeManager.getRoot();
   auto treeRoot = make_shared<WidgetTreeItem>(typeMetaRoot);
   treeRoot->setEnabled(false);
   setRoot(treeRoot);

   //add all children
   std::function<void(std::shared_ptr<WidgetTreeItem>&)> stuffTree = [&](std::shared_ptr<WidgetTreeItem>& currentBranch) {
      for (auto [name, typeMetaChild]: currentBranch->typeMeta->children) {
         //add all children of the current child to the current child's typemeta tree item
         auto newBranch = make_shared<WidgetTreeItem>(typeMetaChild);
         currentBranch->push_back(newBranch);
         stuffTree(newBranch);
         if (typeManager.getType(name)->isVirtual){
            newBranch->setEnabled(false);
         }
      }
   };
   stuffTree(treeRoot);
   cout << treeRoot->typeMeta->typeName << endl;
}

/////////////////////////////////////////////////////////////////////////////////////////
std::optional<std::shared_ptr<Draggable>> WidgetTree::_on_drag_start(ReyEngine::Pos<R_FLOAT> globalPos){
   auto metaAt = getMetaAt(globalToLocal(globalPos));
   if (metaAt) {
      auto typeName = metaAt.value()->item->getText();
      auto typeMeta = Internal::TypeManager::getType(typeName);
      if (typeMeta->isVirtual) return nullopt;
      cout << "drag start at " << globalPos << endl;
      auto label = Label::build("dragLabel");
      label->setText(typeName);
      auto draggable = make_shared<Draggable>(label->getText(), label);
      return draggable;
   }
   return nullopt;
}