#pragma once
#include "Layout.hpp"
#include "SceneTree.h"
#include "WidgetTree.h"


class Editor : public VLayout {
   GFCSDRAW_OBJECT(Editor, VLayout){}
public:
   void _init() override;

private:
   std::shared_ptr<SceneTree> sceneTree;
   std::shared_ptr<WidgetTree> widgetTree;
};