#include "Label.h"
#include "DrawInterface.h"
#include <sstream>
using namespace std;
using namespace GFCSDraw;
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
Label::Label(string name)
: BaseWidget(std::move(name), "Label")
, text("DefaultText")
{}
/////////////////////////////////////////////////////////////////////////////////////////
void Label::render() const {
   _drawText(text.value, {0,0}, 20, BLACK);
}

/////////////////////////////////////////////////////////////////////////////////////////
void Label::_process(float dt) {}

/////////////////////////////////////////////////////////////////////////////////////////
void Label::registerProperties() {
   registerProperty(text);
}

/////////////////////////////////////////////////////////////////////////////////////////
shared_ptr<Label> Label::deserialize(std::string instanceName, PropertyPrototypeMap properties) {
   auto retval = make_shared<Label>(instanceName);
   retval->_deserialize(properties);
   return retval;
}