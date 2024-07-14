#pragma once
#include "ScrollArea.h"
#include "Layout.h"
#include "Workspace.h"

namespace ReyEngine{
   class Inspector : public ReyEngine::ScrollArea {
   public:
      REYENGINE_DEFAULT_BUILD
      REYENGINE_OBJECT_BUILD_ONLY(Inspector, ScrollArea){}
   public:
      void inspect(std::shared_ptr<BaseWidget>); //load an object into the inspector
   protected:
      void _init() override;
   private:
      std::shared_ptr<VLayout> mainVLayout;
   };
}