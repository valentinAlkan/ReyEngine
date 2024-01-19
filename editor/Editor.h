#pragma once
#include "Layout.hpp"
#include "SceneTree.h"
#include "WidgetTree.h"
#include "Inspector.h"

class Editor : public VLayout {
   GFCSDRAW_OBJECT(Editor, VLayout){}

public:
   void _init() override;
//   void inspect(std::shared_ptr<BaseWidget>);
private:
   std::shared_ptr<SceneTree> sceneTree;
   std::shared_ptr<WidgetTree> widgetTree;
   std::shared_ptr<Inspector> inspector;
};