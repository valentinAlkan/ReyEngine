#include "TestWidgets.h"
#include "DrawInterface.h"
#include "Window.h"
#include <sstream>
using namespace std;
using namespace GFCSDraw;
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
PosTestWidget::PosTestWidget(string name)
: BaseWidget(std::move(name), "PosTestWidget")
, someString("someString")
{}

/////////////////////////////////////////////////////////////////////////////////////////
void PosTestWidget::render() const {
Vec2<int> pos = getGlobalPos();
stringstream text;
   text << getName() << "\n";
   text << getPos() << "\n";
   text << "{" + to_string(pos.x) + "," + to_string(pos.y) + "}";
   _drawText(text.str(), {0,0}, 20, RED);
}

/////////////////////////////////////////////////////////////////////////////////////////
void PosTestWidget::_process(float dt) {
   setPos(Window::getMousePos());
}

/////////////////////////////////////////////////////////////////////////////////////////
void PosTestWidget::registerProperties() {
   BaseWidget::registerProperties();
   registerProperty(someString);
}

/////////////////////////////////////////////////////////////////////////////////////////
shared_ptr<PosTestWidget> PosTestWidget::deserialize(std::string instanceName, PropertyPrototypeMap properties) {
   auto retval = make_shared<PosTestWidget>(instanceName);
   retval->_deserialize(properties);
   return retval;
}