#include "TypeTree.h"

using namespace std;
using namespace ReyEngine;
using namespace Internal;
using namespace Tree;

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
      _processList->remove(this, _isProcessed);
   } else {
      _processList->add(this, _isProcessed);
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
Tree::Easable::~Easable(){
   if (_isEased){
      for (auto& easing : _easings){
         _easingList->remove(easing.get(), _isEased);
      }
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
Easing* Tree::Easable::addEasing(std::unique_ptr<Easing>&& easing) {
   _wantsEase = true;
   _easings.push_back(std::move(easing));
   _easingList->add(_easings.back().get(), _isEased);
}