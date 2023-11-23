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
   GFCSDRAW_OBJECT(WidgetTree, Tree){}
public:

protected:
   void _init();
};