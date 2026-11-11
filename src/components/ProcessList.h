#pragma once
#include <algorithm>
#include <functional>
#include <optional>
#include <mutex>
#include <memory>
#include <unordered_set>
#include <vector>

// Everything of a kind that wants processing each frame, processed in the order it was added: the first added goes first,
// whatever the addresses, so a frame runs the same way every time (and a replay plays back as it played).
template <typename T>
class ProcessList {
public:
   ~ProcessList(){clear();}
   static std::optional<T*> add(T* t, bool& isProcessed){
      //should be queued
      std::unique_lock<std::recursive_mutex> lock(instance()._mtx);
      auto retval = instance()._list.insert(t);
      if (retval.second){
         instance()._order.push_back(t);
         isProcessed = true;
         return t;
      }
      return std::nullopt;
   }
   static std::optional<T*> remove(T* t, bool& isProcessed){
      //should be queued
      std::unique_lock<std::recursive_mutex> lock(instance()._mtx);
      auto it = instance()._list.find(t);
      if (it != instance()._list.end()){
         //only remove if found; a pass going on holds its own snapshot, so taking it out of the order is safe
         instance()._list.erase(it);
         std::erase(instance()._order, t);
         isProcessed = !instance()._list.empty();
         return t;
      }
      return std::nullopt;
   }
   static void removeAll(bool& isProcessed){
      std::unique_lock<std::recursive_mutex> lock(instance()._mtx);
      instance()._list.clear();
      instance()._order.clear();
      isProcessed = false;
   }
   static bool contains(T* t){
      std::unique_lock<std::recursive_mutex> lock(instance()._mtx);
      return instance()._list.contains(t);
   }
   static std::optional<T*> find(const T* t){
      std::unique_lock<std::recursive_mutex> lock(instance()._mtx);
      auto it = std::find(instance()._order.begin(), instance()._order.end(), t);
      if (it != instance()._order.end()){
         return *it;
      }
      return std::nullopt;
   }

   template <typename... Args>
   static void processAll(Args... args){
      //recursive so _process can call add/remove on this thread; other threads still wait for the frame
      std::unique_lock<std::recursive_mutex> lock(instance()._mtx);
      auto& self = instance();
      //iterate a snapshot so add/remove during _process can't invalidate the iterator.
      //things added this frame start processing next frame.
      auto snapshot = self.takeSnapshot();
      for (auto& processable : snapshot) {
         //skip anything removed (possibly destroyed) earlier this frame
         if (!self._list.contains(processable)) continue;
         processable->_process(std::forward<Args>(args)...);
      }
      self.giveBack(std::move(snapshot));
   }
   //hands each to a function, in order, as processAll does: for whatever must happen to one before the next is touched
   //(an easing that's ended is finished before the next moves on). Removed ones are skipped; added ones wait a frame.
   static void processEach(const std::function<void(T*)>& each){
      std::unique_lock<std::recursive_mutex> lock(instance()._mtx);
      auto& self = instance();
      auto snapshot = self.takeSnapshot();
      for (auto& processable : snapshot) {
         if (!self._list.contains(processable)) continue;
         each(processable);
      }
      self.giveBack(std::move(snapshot));
   }
   //like processAll, but returns the items whose _process returned true and that are still in the list once the
   //whole frame has run (a later item may have removed an earlier one).
   template <typename... Args>
   static std::vector<T*> processAllDone(Args... args){
      std::unique_lock<std::recursive_mutex> lock(instance()._mtx);
      auto& self = instance();
      auto snapshot = self.takeSnapshot();
      std::vector<T*> done;
      for (auto& processable : snapshot) {
         if (!self._list.contains(processable)) continue;
         if (processable->_process(std::forward<Args>(args)...)) done.push_back(processable);
      }
      self.giveBack(std::move(snapshot));
      std::erase_if(done, [&](T* t){return !self._list.contains(t);});
      return done;
   }
   static void clear(){
      std::unique_lock<std::recursive_mutex> sl(instance()._mtx);
      instance()._list.clear();
      instance()._order.clear();
   }
   static size_t size() {return instance()._list.size();}
protected:
   static ProcessList& instance(){
      static ProcessList instance;
      return instance;
   }
private:
   //the order, copied to pass over. Passes can nest (a _process that runs another), so each has its own; the
   //outermost reuses the spare to avoid reallocating each frame.
   std::vector<T*> takeSnapshot(){
      std::vector<T*> snapshot = std::move(_spare);
      snapshot.assign(_order.begin(), _order.end());
      return snapshot;
   }
   void giveBack(std::vector<T*>&& snapshot){
      snapshot.clear();
      if (snapshot.capacity() > _spare.capacity()) _spare = std::move(snapshot);
   }
   std::unordered_set<T*> _list; //what requires processing, to find quickly
   std::vector<T*> _order;       //the same, in the order they were added: the order they're processed in
   std::recursive_mutex _mtx;
   std::vector<T*> _spare; //a snapshot's room, kept between frames
};