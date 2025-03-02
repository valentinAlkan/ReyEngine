#pragma once
#include "Layout.h"
#include "SceneTree.h"
#include "WidgetTree.h"
#include "Inspector.h"

namespace ReyEngine {
   class Editor : public VLayout {
      REYENGINE_OBJECT_BUILD_ONLY(Editor, VLayout, BaseWidget) {}

   public:
      REYENGINE_DEFAULT_BUILD(Editor);
      void _init() override;
//   void inspect(std::shared_ptr<BaseWidget>);
   private:
      std::shared_ptr<SceneTree> sceneTree;
      std::shared_ptr<WidgetTree> widgetTree;
      std::shared_ptr<Inspector> inspector;
   };
}