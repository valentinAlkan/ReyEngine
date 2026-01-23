#pragma once
#include <new>
#include "Widget.h"

namespace ReyEngine {

   class FocusGroup {
   public:
      void removeFromGroup(std::shared_ptr<Widget>& widget){
         cleanup();
         for (auto it = _widgets.begin(); it != _widgets.end(); /**/){
            if (it->lock() == widget){
               _widgets.erase(it);
               break;
            }
         }
      }
      //return which, if any, item currently has focus
      [[nodiscard]] std::optional<Widget*> getCurrentInFocus() const {
         for (const auto it : _widgets){
            if (auto exists = it.lock()){
               if (exists->isFocused()) return exists.get();
            }
         }
         return {};
      }
      std::optional<Widget*> getCurrentInFocus() {
         cleanup();
         return const_cast<const FocusGroup&>(*this).getCurrentInFocus();
      }


      [[nodiscard]] std::optional<Widget*> getNextInFocus() const {
         //all pointers should be valid here
         if (_widgets.empty()) return {};
         auto inFocus = getCurrentInFocus();
         if (_widgets.size() == 1 || inFocus == nullptr) return _widgets.at(0).lock().get();
         for (auto it = _widgets.begin(); it != _widgets.end(); it++){
            if (auto exists = it->lock()){
               if (exists.get() == inFocus){
                 if (++it == _widgets.end()){
                    return _widgets.front().lock().get();
                 }
                 return it->lock().get();
               }
            }
         }
         return inFocus;
      }
      void setNextInFocus(){
         cleanup();
         if (auto exists = getNextInFocus()){
            exists.value()->setFocused(true);
         }
      }
      void addToGroup(const std::shared_ptr<Widget>& widget){
         cleanup();
         _widgets.push_back(widget);
      }
      void setFocus(Widget* widget){
         cleanup();
         for (auto& it : _widgets){
            if (it.lock().get() == widget){
               widget->setFocused(true);
            }
         }
      }
   private:
      void cleanup(){
         for (auto it = _widgets.begin(); it != _widgets.end(); /**/){
            if (auto exists = it->lock()){
               it++;
            } else {
               it = _widgets.erase(it);
            }
         }
      }
      std::vector<std::weak_ptr<Widget>> _widgets;
   };
}