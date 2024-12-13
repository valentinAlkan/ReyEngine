#pragma once
#include "Component.h"

namespace ReyEngine{
   class Timer : public Internal::Component {
      REYENGINE_DECLARE_STATIC_CONSTEXPR_TYPENAME(Timer)
      Timer(const std::string& instanceName): Internal::Component(instanceName, _get_static_constexpr_typename()){}
   };

}