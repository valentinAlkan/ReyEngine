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