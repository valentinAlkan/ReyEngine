#include "CustomTypes.h"

using namespace std;
using namespace ReyEngine;

void CustomTypes::registerTypes(){
      /* put your custom type registration calls here */
      REGISTER_CUSTOM_TYPE(PosTestWidget, Control, false);
}