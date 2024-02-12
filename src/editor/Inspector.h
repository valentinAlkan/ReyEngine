#pragma once
#include "ScrollArea.hpp"
#include "Layout.h"
#include "Workspace.h"

class Inspector : public ScrollArea {
public:
   REYENGINE_OBJECT(Inspector, ScrollArea){}
public:
   void inspect(std::shared_ptr<BaseWidget>); //load an object into the inspector
protected:
   void _init() override;
private:
   std::shared_ptr<VLayout> mainVLayout;
};