#include "WidgetTree.h"

using namespace std;

void WidgetTree::_init(){
   //populate with widgets
   auto& typeManager = TypeManager::instance();
   auto typeMetaRoot = typeManager.getRoot();
   auto treeRoot = make_shared<WidgetTreeItem>(typeMetaRoot);
   setRoot(treeRoot);

   //add all children
   std::function<void(std::shared_ptr<WidgetTreeItem>&)> stuffTree = [&](std::shared_ptr<WidgetTreeItem>& currentBranch) {
      for (auto [name, typeMetaChild]: currentBranch->typeMeta->children) {
         //add all children of the current child to the current child's typemeta tree item
         auto newBranch = make_shared<WidgetTreeItem>(typeMetaChild);
         currentBranch->push_back(newBranch);
         stuffTree(newBranch);
      }
   };
   stuffTree(treeRoot);
   cout << treeRoot->typeMeta->typeName << endl;
}