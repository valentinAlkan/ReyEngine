#pragma once
#include "Tree.h"
#include "TypeManager.h"

namespace ReyEngine{
   class WidgetTreeItem : public TreeItem{
   public:
      WidgetTreeItem(std::shared_ptr<Internal::TypeMeta> typeMeta): typeMeta(typeMeta), TreeItem(typeMeta->typeName){}
      std::shared_ptr<Internal::TypeMeta> typeMeta;
      void push_back(std::shared_ptr<WidgetTreeItem> item){
         std::shared_ptr<TreeItem> baseptr = std::dynamic_pointer_cast<TreeItem>(item);
         TreeItem::push_back(baseptr);
      };
   };



   class WidgetTree : public Tree{
      REYENGINE_OBJECT_BUILD_ONLY(WidgetTree, Tree){}
   public:
      REYENGINE_DEFAULT_BUILD(WidgetTree);
      std::optional<std::shared_ptr<Draggable>> _on_drag_start(ReyEngine::Pos<R_FLOAT> globalPos) override;
   protected:
      void _init();
   };
}