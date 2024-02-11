#pragma once
#include "Panel.hpp"

//template <typename T>
//class WorkspaceWidget{
//   bool selected = false;
//public:
//   /////////////////////////////////////////////////////////////////////////////////////////
//   void renderChain(ReyEngine::Pos<double>& parentOffset) {
//      stastic_assert(std::is_base_of_v<T, BaseWidget>());
//      auto& bw = static_cast<BaseWidget&>(*this);
//      if (!bw._visible) return;
//      ReyEngine::Pos<double> localOffset;
//      bw.renderBegin(localOffset);
//      bw._renderOffset += (localOffset + parentOffset);
//      bw.render();
//      //renderChildren
//      for (const auto& [name, childIter] : bw._children){
//         childIter.second->renderChain(bw._renderOffset);
//      }
//      bw._renderOffset -= (localOffset + parentOffset); //subtract local offset when we are done
//      bw.renderEnd();
//      bw.renderEditorFeatures();
//   }
//   static std::shared_ptr<WorkspaceWidget<BaseWidget>> _deserialize(const std::string& instanceName, PropertyPrototypeMap& properties) {
//      CTOR_RECT = {0,0,0,0};
//      auto retval = std::make_shared<WorkspaceWidget>(instanceName, r);
//      retval->BaseWidget::_deserializeEditor(properties);
//      return retval;
//   }
//};

class WorkspaceWidget : public BaseWidget {
   
};


class Workspace : public Panel {
   REYENGINE_OBJECT(Workspace, Panel){}
public:
   struct EventWidgetAdded : public Event<EventWidgetAdded> {
      EVENT_CTOR_SIMPLE(EventWidgetAdded, Event<EventWidgetAdded>){}
      std::shared_ptr<BaseWidget> widget;
   };
   void _init();
   Handled _on_drag_drop(std::shared_ptr<Draggable>) override;
   void _on_child_added(std::shared_ptr<BaseWidget>&) override;
//   std::shared_ptr<WorkspaceWidget> instanceWidget(std::shared_ptr<BaseWidget>);
};
