#pragma once
#include "Panel.hpp"

class Workspace : public Panel {
   GFCSDRAW_OBJECT(Workspace, Panel){}
public:
   void _init();
   Handled _on_drag_drop(std::shared_ptr<Draggable>) override;
   void _on_child_added(std::shared_ptr<BaseWidget>&) override;
};
