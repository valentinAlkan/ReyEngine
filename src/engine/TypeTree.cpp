#include "TypeTree.h"

using namespace std;
using namespace ReyEngine;
using namespace Internal;

std::unique_ptr<Internal::Tree::ProcessList> ReyEngine::Internal::Tree::ProcessList::ProcessList::_processList;
/////////////////////////////////////////////////////////////////////////////////////////
Tree::ProcessList& Tree::ProcessList::instance(){
   if (!_processList){
      _processList = make_unique<ProcessList>();
   }
   return *_processList;
}

/////////////////////////////////////////////////////////////////////////////////////////
Tree::Processable::~Processable(){
   _wantsProcess = false;
   if (_isProcessed){
      setProcess(false);
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
void Tree::Processable::setProcess(bool value) {
   _wantsProcess = value;
   if (_isProcessed) {
      ProcessList::remove(this);
   } else {
      ProcessList::add(this);
   }
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
std::optional<Tree::Processable*> Tree::ProcessList::add(Processable* processable) {
   //should be queued
   unique_lock<mutex> lock(instance()._mtx);
   auto retval = instance()._list.insert(processable);
   if (retval.second){
      processable->_isProcessed = true;
      return processable;
   }
   return nullopt;
}

///////////////////////////////////////////////////////////////////////////////////////////
std::optional<Tree::Processable*> Tree::ProcessList::remove(Processable* processable) {
   //should be queued
   unique_lock<mutex> lock(instance()._mtx);
   auto it = instance()._list.find(processable);
   if (it != instance()._list.end()){
      //only remove if found;
      instance()._list.erase(it);
      processable->_isProcessed = false;
      return processable;
   }
   return nullopt;
}

///////////////////////////////////////////////////////////////////////////////////////////
void Tree::ProcessList::processAll(R_FLOAT dt) {
   unique_lock<mutex> lock(instance()._mtx);
   for (auto& processable : instance()._list) {
      processable->_process(dt);
   }
}

///////////////////////////////////////////////////////////////////////////////////////////
std::optional<Tree::Processable*> Tree::ProcessList::find(const Processable* processable) {
   auto it = std::find(instance()._list.begin(), instance()._list.end(), processable);
   if (it != instance()._list.end()){
      return *it;
   }
   return nullopt;
}