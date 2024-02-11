#pragma once
#include "Tree.h"
#include "TypeManager.h"

class WidgetTreeItem : public TreeItem{
public:
   WidgetTreeItem(std::shared_ptr<TypeMeta> typeMeta): typeMeta(typeMeta), TreeItem(typeMeta->typeName){}
   std::shared_ptr<TypeMeta> typeMeta;
   void push_back(std::shared_ptr<WidgetTreeItem> item){
      std::shared_ptr<TreeItem> baseptr = std::dynamic_pointer_cast<TreeItem>(item);
      TreeItem::push_back(baseptr);
   };
};



class WidgetTree : public Tree{
   REYENGINE_OBJECT(WidgetTree, Tree){}
public:
   std::optional<std::shared_ptr<Draggable>> _on_drag_start(ReyEngine::Pos<int> globalPos) override;
protected:
   void _init();
};