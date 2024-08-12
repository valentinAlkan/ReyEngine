#pragma once
#include "Panel.h"

class SuperPanel : public ReyEngine::Panel {
   //this macro is necessary unless you put a bunch of boilerplate in your code.
   // What it does is declare some constructors and a static constexpr TYPE_NAME that the type system uses
   REYENGINE_OBJECT_BUILD_ONLY(SuperPanel, Panel){}
public:
   REYENGINE_DEFAULT_BUILD(SuperPanel)
};