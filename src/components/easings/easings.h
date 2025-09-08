//carefully swiped from easings.net
#include <StrongUnits.h>
#include <cmath>
#ifndef EASINGS_FLOAT_TYPE
#define EASINGS_FLOAT_TYPE float 
#endif

/////////////////////////////////////////////////////////////////////////////////////////
static constexpr inline Fraction easeOutBounce(EASINGS_FLOAT_TYPE x);

/////////////////////////////////////////////////////////////////////////////////////////
static constexpr inline Fraction easeInSine(EASINGS_FLOAT_TYPE x) {
   return 1 - std::cos((x * M_PI) / 2);
}

/////////////////////////////////////////////////////////////////////////////////////////
static constexpr inline Fraction easeOutSine(EASINGS_FLOAT_TYPE x) {
   return std::sin((x * M_PI) / 2);
}

/////////////////////////////////////////////////////////////////////////////////////////
static constexpr inline Fraction easeInOutSine(EASINGS_FLOAT_TYPE x) {
   return -(std::cos(M_PI * x) - 1) / 2;
}

/////////////////////////////////////////////////////////////////////////////////////////
static constexpr inline Fraction easeInQuad(EASINGS_FLOAT_TYPE x) {
   return x * x;
}

/////////////////////////////////////////////////////////////////////////////////////////
static constexpr inline Fraction easeOutQuad(EASINGS_FLOAT_TYPE x) {
   return 1 - (1 - x) * (1 - x);
}

/////////////////////////////////////////////////////////////////////////////////////////
static constexpr inline Fraction easeInOutQuad(EASINGS_FLOAT_TYPE x) {
   return x < 0.5 ? 2 * x * x : 1 - std::pow(-2 * x + 2, 2) / 2;
}

/////////////////////////////////////////////////////////////////////////////////////////
static constexpr inline Fraction easeInCubic(EASINGS_FLOAT_TYPE x) {
   return x * x * x;
}

/////////////////////////////////////////////////////////////////////////////////////////
static constexpr inline Fraction easeOutCubic(EASINGS_FLOAT_TYPE x) {
   return 1 - std::pow(1 - x, 3);
}

/////////////////////////////////////////////////////////////////////////////////////////
static constexpr inline Fraction easeInOutCubic(EASINGS_FLOAT_TYPE x) {
   return x < 0.5 ? 4 * x * x * x : 1 - std::pow(-2 * x + 2, 3) / 2;
}

/////////////////////////////////////////////////////////////////////////////////////////
static constexpr inline Fraction easeInQuart(EASINGS_FLOAT_TYPE x) {
   return x * x * x * x;
}

/////////////////////////////////////////////////////////////////////////////////////////
static constexpr inline Fraction easeOutQuart(EASINGS_FLOAT_TYPE x) {
   return 1 - std::pow(1 - x, 4);
}

/////////////////////////////////////////////////////////////////////////////////////////
static constexpr inline Fraction easeInOutQuart(EASINGS_FLOAT_TYPE x) {
   return x < 0.5 ? 8 * x * x * x * x : 1 - std::pow(-2 * x + 2, 4) / 2;
}

/////////////////////////////////////////////////////////////////////////////////////////
static constexpr inline Fraction easeInQuint(EASINGS_FLOAT_TYPE x) {
   return x * x * x * x * x;
}

/////////////////////////////////////////////////////////////////////////////////////////
static constexpr inline Fraction easeOutQuint(EASINGS_FLOAT_TYPE x) {
   return 1 - std::pow(1 - x, 5);
}

/////////////////////////////////////////////////////////////////////////////////////////
static constexpr inline Fraction easeInOutQuint(EASINGS_FLOAT_TYPE x) {
   return x < 0.5 ? 16 * x * x * x * x * x : 1 - std::pow(-2 * x + 2, 5) / 2;
}

/////////////////////////////////////////////////////////////////////////////////////////
static constexpr inline Fraction easeInExpo(EASINGS_FLOAT_TYPE x) {
   return x == 0 ? 0 : std::pow(2, 10 * x - 10);
}

/////////////////////////////////////////////////////////////////////////////////////////
static constexpr inline Fraction easeOutExpo(EASINGS_FLOAT_TYPE x) {
   return x == 1 ? 1 : 1 - std::pow(2, -10 * x);
}

/////////////////////////////////////////////////////////////////////////////////////////
static constexpr inline Fraction easeInOutExpo(EASINGS_FLOAT_TYPE x) {
   return x == 0
          ? 0
          : x == 1
            ? 1
            : x < 0.5 ? std::pow(2, 20 * x - 10) / 2
                      : (2 - std::pow(2, -20 * x + 10)) / 2;
}

/////////////////////////////////////////////////////////////////////////////////////////
static constexpr inline Fraction easeInCirc(EASINGS_FLOAT_TYPE x) {
   return 1 - std::sqrt(1 - std::pow(x, 2));
}

/////////////////////////////////////////////////////////////////////////////////////////
static constexpr inline Fraction easeOutCirc(EASINGS_FLOAT_TYPE x) {
   return std::sqrt(1 - std::pow(x - 1, 2));
}

/////////////////////////////////////////////////////////////////////////////////////////
static constexpr inline Fraction easeInOutCirc(EASINGS_FLOAT_TYPE x) {
   return x < 0.5
          ? (1 - std::sqrt(1 - std::pow(2 * x, 2))) / 2
          : (std::sqrt(1 - std::pow(-2 * x + 2, 2)) + 1) / 2;
}

/////////////////////////////////////////////////////////////////////////////////////////
static constexpr inline Fraction easeInBack(EASINGS_FLOAT_TYPE x) {
   const EASINGS_FLOAT_TYPE c1 = 1.70158;
   const EASINGS_FLOAT_TYPE c3 = c1 + 1;
   return c3 * x * x * x - c1 * x * x;
}

/////////////////////////////////////////////////////////////////////////////////////////
static constexpr inline Fraction easeOutBack(EASINGS_FLOAT_TYPE x) {
   const EASINGS_FLOAT_TYPE c1 = 1.70158;
   const EASINGS_FLOAT_TYPE c3 = c1 + 1;
   return 1 + c3 * std::pow(x - 1, 3) + c1 * std::pow(x - 1, 2);
}

/////////////////////////////////////////////////////////////////////////////////////////
static constexpr inline Fraction easeInOutBack(EASINGS_FLOAT_TYPE x) {
   const EASINGS_FLOAT_TYPE c1 = 1.70158;
   const EASINGS_FLOAT_TYPE c2 = c1 * 1.525;
   return x < 0.5
          ? (std::pow(2 * x, 2) * ((c2 + 1) * 2 * x - c2)) / 2
          : (std::pow(2 * x - 2, 2) * ((c2 + 1) * (x * 2 - 2) + c2) + 2) / 2;
}

/////////////////////////////////////////////////////////////////////////////////////////
static constexpr inline Fraction easeInElastic(EASINGS_FLOAT_TYPE x) {
   const EASINGS_FLOAT_TYPE c4 = (2 * M_PI) / 3;
   return x == 0
          ? 0
          : x == 1
            ? 1
            : -std::pow(2, 10 * x - 10) * std::sin((x * 10 - 10.75) * c4);
}

/////////////////////////////////////////////////////////////////////////////////////////
static constexpr inline Fraction easeOutElastic(EASINGS_FLOAT_TYPE x) {
   const EASINGS_FLOAT_TYPE c4 = (2 * M_PI) / 3;
   return x == 0
          ? 0
          : x == 1
            ? 1
            : std::pow(2, -10 * x) * std::sin((x * 10 - 0.75) * c4) + 1;
}

/////////////////////////////////////////////////////////////////////////////////////////
static constexpr inline Fraction easeInOutElastic(EASINGS_FLOAT_TYPE x) {
   const EASINGS_FLOAT_TYPE c5 = (2 * M_PI) / 4.5;
   return x == 0
          ? 0
          : x == 1
            ? 1
            : x < 0.5
              ? -(std::pow(2, 20 * x - 10) * std::sin((20 * x - 11.125) * c5)) / 2
              : (std::pow(2, -20 * x + 10) * std::sin((20 * x - 11.125) * c5)) / 2 + 1;
}

/////////////////////////////////////////////////////////////////////////////////////////
static constexpr inline Fraction easeInBounce(EASINGS_FLOAT_TYPE x) {
   return 1 - easeOutBounce(1 - x);
}

/////////////////////////////////////////////////////////////////////////////////////////
static constexpr inline Fraction easeOutBounce(EASINGS_FLOAT_TYPE x) {
   const EASINGS_FLOAT_TYPE n1 = 7.5625;
   const EASINGS_FLOAT_TYPE d1 = 2.75;

   if (x < 1 / d1) {
      return n1 * x * x;
   } else if (x < 2 / d1) {
      x -= 1.5 / d1;
      return n1 * x * x + 0.75;
   } else if (x < 2.5 / d1) {
      x -= 2.25 / d1;
      return n1 * x * x + 0.9375;
   } else {
      x -= 2.625 / d1;
      return n1 * x * x + 0.984375;
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
static constexpr inline Fraction easeInOutBounce(EASINGS_FLOAT_TYPE x) {
   return x < 0.5
          ? (1 - easeOutBounce(1 - 2 * x)) / 2
          : (1 + easeOutBounce(2 * x - 1)) / 2;
}