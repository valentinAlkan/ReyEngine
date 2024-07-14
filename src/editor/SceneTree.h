#pragma once
#include "Tree.h"

namespace ReyEngine {
   class SceneTree : public Tree{
      REYENGINE_DEFAULT_BUILD;
      REYENGINE_OBJECT_BUILD_ONLY(SceneTree, Tree){}
   public:
   };

   class SceneTreeItem : public TreeItem {

   };
}