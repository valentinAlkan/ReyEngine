#pragma once
#include "ReyEngine.h"
namespace ReyEngine {

    template <typename T>
    struct Pos3 : Vec3<T> {

    };

    template <typename T>
    struct Cube {
        T x;
        T y;
        T z;
        T xWidth;
        T yHeight;
        T zDepth;
    };
}