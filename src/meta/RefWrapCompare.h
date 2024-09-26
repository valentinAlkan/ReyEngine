#pragma once
#include <iostream>

struct RefWrapperCompare {
   template<typename T>
   bool operator()(const std::reference_wrapper<T>& lhs, const std::reference_wrapper<T>& rhs) const {
      return &(lhs.get()) < &(rhs.get());
   }
};