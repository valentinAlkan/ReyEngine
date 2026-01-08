#pragma once
#include <optional>
#include <mutex>
#include <memory>
#include <unordered_set>

template <typename T>
class ProcessList {
public:
   ~ProcessList(){clear();}
   static std::optional<T*> add(T* t, bool& isProcessed){
      //should be queued
      std::unique_lock<std::mutex> lock(instance()._mtx);
      auto retval = instance()._list.insert(t);
      if (retval.second){
         isProcessed = true;
         //invalidate iterator
         instance()._it = instance()._list.begin();
         return t;
      }
      return std::nullopt;
   }
   static std::optional<T*> remove(T* t, bool& isProcessed){
      //should be queued
      std::unique_lock<std::mutex> lock(instance()._mtx);
      auto it = instance()._list.find(t);
      if (it != instance()._list.end()){
         //only remove if found;
         instance()._list.erase(it);
         isProcessed = !instance()._list.empty();
         //invalidate iterator
         instance()._it = instance()._list.begin();
         return t;
      }
      return std::nullopt;
   }
   static void removeAll(bool& isProcessed){
      std::unique_lock<std::mutex> lock(instance()._mtx);
      instance()._list.clear();
      isProcessed = false;
      //invalidate iterator
      instance()._it = instance()._list.begin();
   }
   static std::optional<T*> find(const T* t){
      auto it = std::find(instance()._list.begin(), instance()._list.end(), t);
      if (it != instance()._list.end()){
         return *it;
      }
      return std::nullopt;
   }

   template <typename... Args>
   static void processAll(Args... args){
      std::unique_lock<std::mutex> lock(instance()._mtx);
      for (auto& processable : instance()._list) {
         processable->_process(std::forward<Args>(args)...);
      }
   }
   //returns whichever T was just processed
   template <typename... Args>
   static T* processfirst(Args... args){
      auto& it = instance()._it;
      auto& list = instance()._list;
      if (list.empty()) return nullptr;
      std::unique_lock<std::mutex> lock(instance()._mtx);
      it = list.begin();
      (*it)->_process(std::forward<Args>(args)...);
      auto retval = *it;
      ++it;
      return retval;
   }
   template <typename... Args>
   static T* processNext(Args... args){
      auto& it = instance()._it;
      auto& list = instance()._list;
      if (list.empty()) return nullptr;
      std::unique_lock<std::mutex> lock(instance()._mtx);
      if (it == list.end()) return nullptr;
      (*it)->_process(std::forward<Args>(args)...);
      auto retval = *it;
      ++it;
      return retval;
   }
   static void clear(){
      std::unique_lock<std::mutex> sl(instance()._mtx);
      instance()._list.clear();
      instance()._it = instance()._list.begin();
   }
   static size_t size() {return instance()._processList->_list.size();}
protected:
   static ProcessList& instance(){
      static ProcessList instance;
      return instance;
   }
private:
   std::unordered_set<T*> _list; //list of things that require processing. No specific order.
   std::mutex _mtx;
   std::unordered_set<T*>::iterator _it;
};