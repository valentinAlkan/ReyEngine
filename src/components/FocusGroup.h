#pragma once
#include <new>
#include "Widget.h"

namespace ReyEngine {

   class FocusGroup {
   protected:
      void removeFromGroup(Widget* widget){
         for (auto it = _widgets.begin(); it != _widgets.end(); it++){
            if (*it == widget){
               _widgets.erase(it);
               break;
            }
         }
      }
      struct LifeTimeManager{
         LifeTimeManager(FocusGroup* group, LifeTimeManager* buddy, Widget* widget)
         : _group(group)
         , _widget(widget)
         , _buddy(buddy)
         {
            group->removeFromGroup(widget);
            group->_widgets.push_back(widget);

         }
         ~LifeTimeManager(){
            //tell our buddy we're dyin
            if (_buddy) _buddy->_buddy = nullptr;
            else _group->removeFromGroup(_widget);
         }
         FocusGroup* _group;
         Widget* _widget;
         LifeTimeManager* _buddy;
         int id;
      };
      Widget* getCurrentInFocus(){}
      Widget* getNextInFocus(){}
      std::unique_ptr<LifeTimeManager> addWidget(Widget* widget){
//         alignas(LifeTimeManager) unsigned char buffer[sizeof(LifeTimeManager)];
//         LifeTimeManager* b_raw = reinterpret_cast<LifeTimeManager*>(buffer);
//         auto unique_a = std::make_unique<LifeTimeManager>(this, b, widget);
//
//         //now we have to use placement new to create a lifetime manager at addr b since that is where a expects it to be
//
//         LifeTimeManager* b = new (buffer) LifeTimeManager();
//
//
//         auto unique_b = std::make_unique<LifeTimeManager>(this, a, widget);

      }
   private:
      std::vector<Widget*> _widgets;
      Widget* _focused = nullptr;
      std::mutex _mtx;
      int id = 0;
   };
}