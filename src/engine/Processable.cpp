#include "Processable.h"
#include "Application.h"
#include "Window.h"

using namespace ReyEngine;
using namespace Internal;

/////////////////////////////////////////////////////////////////////////////////////////
void Processable::setProcess(bool value) {
   Application::getWindow(0).setProcess(value, this);
}

/////////////////////////////////////////////////////////////////////////////////////////
bool Processable::isProcessed() const {
   return Application::getWindow(0).isProcessed(this);
}