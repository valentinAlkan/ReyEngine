#include "TestWidgets.h"
#include "DrawInterface.h"
#include "Window.h"
#include <sstream>
using namespace std;
using namespace GFCSDraw;
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
void PosTestWidget::render() const {
Vec2<int> pos = getGlobalPos();
stringstream text;
   text << getName() << "\n";
   text << getPos() << "\n";
   text << getRect().toString() << "\n";
   text << "someString = " << someString.value << "\n";
   text << "subProp = " << someString.subProp.value << "\n";
   _drawText(text.str(), {0,0}, 20, Colors::red);
}

/////////////////////////////////////////////////////////////////////////////////////////
void PosTestWidget::_process(float dt) {
   setPos(Window::getMousePos());
}

/////////////////////////////////////////////////////////////////////////////////////////
void PosTestWidget::registerProperties() {
   registerProperty(someString);
}