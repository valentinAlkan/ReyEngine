#pragma once
#include "Layout.h"
#include "SceneTree.h"
#include "WidgetTree.h"
#include "Inspector.h"

class Editor : public VLayout {
   REYENGINE_OBJECT(Editor, VLayout){}

public:
   void _init() override;
//   void inspect(std::shared_ptr<BaseWidget>);
private:
   std::shared_ptr<SceneTree> sceneTree;
   std::shared_ptr<WidgetTree> widgetTree;
   std::shared_ptr<Inspector> inspector;
};