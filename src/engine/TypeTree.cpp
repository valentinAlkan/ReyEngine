#include "TypeTree.h"
#include "Window.h"

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
   if (value == _isProcessed) return; //already in the requested state
   if (value) {
      ProcessList<Processable>::add(this, _isProcessed);
   } else {
      ProcessList<Processable>::remove(this, _isProcessed);
      //remove() reports back whether the *global* list still has entries, which says nothing
      //about us - we were just taken out of it, so record that directly.
      _isProcessed = false;
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
void Windowable::updateTree() const {
   if (_window) _window->_on_tree_updated();
}
