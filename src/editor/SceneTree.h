#pragma once
#include "Tree.h"

namespace ReyEngine {
   class SceneTree : public Tree{
      REYENGINE_OBJECT_BUILD_ONLY(SceneTree, Tree){}
   public:
      REYENGINE_DEFAULT_BUILD(SceneTree);
   };

   class SceneTreeItem : public TreeItem {

   };
}