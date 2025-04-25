#pragma once
#include <ratio>
#include <functional>
#include <type_traits>
#include <cmath>

namespace Units {
   template<typename T, template<typename> class crtpType>
   struct crtp {
      constexpr T& underlying() { return static_cast<T &>(*this); }
      constexpr const T& underlying() const { return static_cast<T const &>(*this); }
   };
}

template <typename T, typename Parameter, typename Ratio, template<typename> class... Skills>
class NamedTypeImpl;

template <typename T, typename Parameter, template<typename> class... Skills>
using NamedType = NamedTypeImpl<T, Parameter, std::ratio<1>, Skills...>;

template <typename T> struct ToDouble : Units::crtp<T, ToDouble> {explicit constexpr operator double() const { return static_cast<double>(this->underlying().get());}};
template <typename T> struct ToInt : Units::crtp<T, ToInt>       {explicit constexpr operator double() const { return static_cast<int>(this->underlying().get());}};
template <typename T> struct ToFloat : Units::crtp<T, ToFloat>   {explicit constexpr operator double() const { return static_cast<float>(this->underlying().get());}};

namespace Units {
   template<typename T, typename Parameter, typename Ratio, template<typename> class... Skills>
   std::ostream &operator<<(std::ostream &os, NamedTypeImpl<T, Parameter, Ratio, Skills...> const &object) {
      os << object.get();
      return os;
   }

}

////////////////////////////////////////////////////////////////////////////////
template <typename T, typename Parameter, typename Ratio, template<typename> class... Skills>
class NamedTypeImpl : public Skills<NamedTypeImpl<T, Parameter, Ratio, Skills...>>...
{
public:
   // Literal detection helper
   template <typename U>
   static constexpr bool is_literal(U) {
      return std::is_constant_evaluated();
   }

   // Special case for literals of this type
   template <auto Value>
   static constexpr bool is_literal_of_type = true;


   // Add inside NamedTypeImpl class
   template <typename... Args>
   static constexpr bool are_all_literals(Args&&... args) {
      // Use a dummy lambda that depends on each parameter
      auto dummy = [](auto&& arg) {
         (void)arg; // Touch the parameter without using it
         return true;
      };

      // The fold expression uses the parameter pack
      (... && dummy(args));

      // Then check if in a constexpr context
      return std::is_constant_evaluated();
   }



   constexpr NamedTypeImpl(){
      //initialize to zero, if possible
      if constexpr (requires { T(0); }) {
         _value = T(0);
      }
   }
   constexpr NamedTypeImpl(T const& value) : _value(value) {}
   constexpr NamedTypeImpl(const NamedTypeImpl& other): _value(other._value){}
//   template<typename T_ = T> constexpr NamedTypeImpl(T&& value, typename std::enable_if<!std::is_reference<T_>{}, std::nullptr_t>::type = nullptr) : _value(std::move(value)) {}

   // get
   constexpr T& get() { return _value; }
   [[nodiscard]] constexpr T const& get() const {return _value; }

   // conversions with ratios
   template <typename Ratio2>
   constexpr operator NamedTypeImpl<T, Parameter, Ratio2, Skills...>() const
   {
      return NamedTypeImpl<T, Parameter, Ratio2, Skills...>(get() * Ratio::num / Ratio::den * Ratio2::den / Ratio2::num);
   }
   template<typename Ratio1, typename Ratio2>
   using MultiplyRatio = std::ratio<Ratio1::num * Ratio2::num, Ratio1::den * Ratio2::den>;
   template <typename Ratio2>
   using GetMultiple = NamedTypeImpl<T, Parameter, MultiplyRatio<Ratio, Ratio2>, Skills...>;

   //assignment overloads
   constexpr NamedTypeImpl& operator=(const NamedTypeImpl& other){ _value = other._value; return *this;}
   constexpr NamedTypeImpl& operator=(double d)                  { _value = d; return *this;}

   //conversion booleans
   constexpr bool operator==(const NamedTypeImpl& other) const {return _value == NamedTypeImpl(other)._value;}
   constexpr bool operator!=(const NamedTypeImpl& other) const {return _value != NamedTypeImpl(other)._value;}
   constexpr bool operator >(const NamedTypeImpl& other) const {return _value > NamedTypeImpl(other)._value;}
   constexpr bool operator <(const NamedTypeImpl& other) const {return _value < NamedTypeImpl(other)._value;}
   constexpr bool operator>=(const NamedTypeImpl& other) const {return _value >= NamedTypeImpl(other)._value;}
   constexpr bool operator<=(const NamedTypeImpl& other) const {return _value <= NamedTypeImpl(other)._value;}

   //addition + subtraction
   constexpr NamedTypeImpl  operator+(NamedTypeImpl const& other) const {return _value +  NamedTypeImpl(other)._value;}
   constexpr NamedTypeImpl& operator+=(NamedTypeImpl const& other)      {       _value += NamedTypeImpl(other)._value; return *this;}
   constexpr NamedTypeImpl  operator-(NamedTypeImpl const& other) const {return _value -  NamedTypeImpl(other)._value;}
   constexpr NamedTypeImpl& operator-=(NamedTypeImpl const& other)      {       _value -= NamedTypeImpl(other)._value; return *this; }

   friend constexpr NamedTypeImpl operator+(double d, NamedTypeImpl const& other) {return d + other._value;}
   friend constexpr NamedTypeImpl operator-(double d, NamedTypeImpl const& other) {return d - other._value;}

   //scalar arithmetic
   constexpr NamedTypeImpl operator+(double d) const {return _value + d;}
   constexpr NamedTypeImpl& operator+=(double d)     {_value += d; return *this;}
   constexpr NamedTypeImpl operator-(double d) const {return _value - d;}
   constexpr NamedTypeImpl& operator-=(double d)     {_value -= d; return *this;}
   constexpr NamedTypeImpl operator/(double d) const {return _value / d;}
   constexpr NamedTypeImpl& operator/=(double d)     {_value /= d; return *this;}
   constexpr NamedTypeImpl operator*(double d) const {return _value * d;}
   constexpr NamedTypeImpl& operator*=(double d)     {_value -= d; return *this;}
   constexpr NamedTypeImpl operator%(int i)       {return (int)_value % i;}

   friend constexpr NamedTypeImpl operator/(double d, NamedTypeImpl const& other) {return other._value / d;}
   friend constexpr NamedTypeImpl operator*(double d, NamedTypeImpl const& other) {return other._value * d;}
   friend constexpr NamedTypeImpl operator%(double d, NamedTypeImpl const& other) {return other._value % d;}

   //scalar booleans
   constexpr bool operator==(double d) const {return _value == d;}
   constexpr bool operator!=(double d) const {return _value != d;}
   constexpr bool operator >(double d) const {return _value  > d;}
   constexpr bool operator <(double d) const {return _value  < d;}
   constexpr bool operator>=(double d) const {return _value >= d;}
   constexpr bool operator<=(double d) const {return _value <= d;}
   friend constexpr bool operator==(double lhs, NamedTypeImpl const& rhs) {return lhs == rhs._value;}
   friend constexpr bool operator!=(double lhs, NamedTypeImpl const& rhs) {return lhs != rhs._value;}
   friend constexpr bool operator >(double lhs, NamedTypeImpl const& rhs) {return lhs > rhs._value;}
   friend constexpr bool operator <(double lhs, NamedTypeImpl const& rhs) {return lhs < rhs._value;}
   friend constexpr bool operator>=(double lhs, NamedTypeImpl const& rhs) {return lhs >= rhs._value;}
   friend constexpr bool operator<=(double lhs, NamedTypeImpl const& rhs) {return lhs <= rhs._value;}

   //unary operators
   constexpr NamedTypeImpl operator-() const {return -_value;}
   constexpr NamedTypeImpl operator+() const {return _value;}

   template <typename NamedType>
   constexpr NamedType toType() const{
      //does not do conversion!!!!
      return NamedType(_value);
   }

   constexpr bool isNan(){return std::isnan(_value);}
protected:
   T _value;
};

//add std::namespace junk here
namespace std {
   template<typename NamedType> constexpr bool isfinite(const NamedType &v) {return std::isfinite(v.get());}
   template<typename NamedType> constexpr NamedType abs(const NamedType &v) {return std::abs(v.get());}
};

template <typename T, typename Parameter, template<typename> class... Skills>
using NamedType = NamedTypeImpl<T, Parameter, std::ratio<1>, Skills...>;

template <typename StrongType, typename Ratio>
using MultipleOf = typename StrongType::template GetMultiple<Ratio>;

template<template<typename T> class StrongType, typename T>
StrongType<T> make_named(T const& value)
{
   return StrongType<T>(value);
}

namespace StrongUnitParameters{
   struct MeterParameter{};
   struct RadiansParameter{};
   struct MetersHAEParameter{};
   struct MetersMSLParameter{};
   struct MetersPerSecondParameter{};
   struct Fraction{};
}

//our implementations
using Meters = NamedType<double, StrongUnitParameters::MeterParameter, ToDouble>;
using Kilometers = MultipleOf<Meters, std::kilo>;
using Feet = MultipleOf<Meters, std::ratio<3048, 10000>>;
using Inches = MultipleOf<Feet, std::ratio<1, 12>>;
constexpr Meters operator"" _m(unsigned long long value){return {(double)value};}
constexpr Kilometers operator"" _km(unsigned long long value){return {(double)value};}
constexpr Feet operator"" _ft(unsigned long long value){return {(double)value};}
constexpr Inches operator"" _in(unsigned long long value){return {(double)value};}
constexpr Meters operator"" _m(long double value){return {(double)value};}
constexpr Kilometers operator"" _km(long double value){return {(double)value};}
constexpr Feet operator"" _ft(long double value){return {(double)value};}
constexpr Inches operator"" _in(long double value){return {(double)value};}

using Radians = NamedType<double, StrongUnitParameters::RadiansParameter, ToDouble>;
using Degrees = MultipleOf<Radians, std::ratio<31415926535897932, 1800000000000000000>>;
using Milliradians = MultipleOf<Radians, std::milli>;

constexpr Radians       operator"" _rad(unsigned long long value){return {(double)value};}
constexpr Degrees       operator"" _deg(unsigned long long value){return {(double)value};}
constexpr Milliradians  operator"" _mrad(unsigned long long value){return {(double)value};}
constexpr Radians       operator"" _rad(long double value){return {(double)value};}
constexpr Degrees       operator"" _deg(long double value){return {(double)value};}
constexpr Milliradians  operator"" _mrad(long double value){return {(double)value};}

using MetersMSL = NamedType<double, StrongUnitParameters::MetersMSLParameter, ToDouble>; //does not implicitly convert to meters
using KilometersMSL = MultipleOf<MetersMSL, std::kilo>;
using MetersHAE = NamedType<double, StrongUnitParameters::MetersHAEParameter, ToDouble>; //does not implicitly convert to meters
constexpr MetersMSL  operator"" _m_msl(unsigned long long value){return {(double)value};}
constexpr MetersHAE  operator"" _m_hae(unsigned long long value){return {(double)value};}
constexpr MetersHAE  operator"" _m_hae(long double value){return {(double)value};}
constexpr MetersMSL  operator"" _m_msl(long double value){return {(double)value};}

using MetersPerSecond = NamedType<double, StrongUnitParameters::MetersPerSecondParameter, ToDouble>;
using Knots = MultipleOf<MetersPerSecond, std::ratio<100000000000, 194384449244>>;

using Fraction = NamedType<double, StrongUnitParameters::Fraction, ToDouble>;
using Perdeca = MultipleOf<Fraction, std::deci>;
using Percent = MultipleOf<Fraction, std::centi>;
using Permille = MultipleOf<Fraction, std::milli>;
constexpr Percent       operator"" _pct(unsigned long long value){return {(double)value};}
constexpr Percent       operator"" _pct(long double value){return {(double)value};}

template<> template<>
constexpr Meters MetersHAE::toType<Meters>() const {
   return _value;
}

template<> template<>
constexpr Meters MetersMSL::toType<Meters>() const {
   return _value;
}

//check our work
#define ft Feet(3048_m)
#define pi2Rad (M_PI * 2_rad)
#define three_sixty_deg 360_deg
static_assert(ft == 10000);
static_assert(ft != 3);
static_assert(ft != 2);
static_assert(ft != 1);
static_assert(ft != 0);
static_assert(ft != -1);
static_assert(ft != -2);
static_assert(ft != -3);
static_assert(ft == Feet(10000));
static_assert(Feet(1000) == 1000_ft);
static_assert(Feet(1000) + 1_ft == 1001_ft);
static_assert(1_km - 1000_m == 0);
static_assert(1_km + 1000_m == 2_km);
static_assert((1 + ft) == 10001);
static_assert((ft + 2) == 10002);
static_assert((2 + ft) == 10002);
static_assert((ft - 1) == 9999);
static_assert((1 - ft) == -9999);
static_assert((ft - 2) == 9998);
static_assert((2 - ft) == -9998);
static_assert((ft / 2) == 5000);
static_assert((2 / ft) == 5000);
static_assert((ft * 2) == 20000);
static_assert((2 * ft) == 20000);
static_assert(1000_m == 1000_m);
static_assert((1000_m).get() == 1000);
static_assert((1_km).get() != 1000);
static_assert((0.445_km).get() == 0.445);
static_assert(1000_m == 1_km);
static_assert(999_m < 1_km);
static_assert(1001_m > 1_km);
static_assert(9999_ft < 3048_m);
static_assert(10001_ft > 3048_m);
static_assert(three_sixty_deg > Degrees(359.9999999));
static_assert(pi2Rad < Degrees(360.0000001));
static_assert(three_sixty_deg/2 >= Degrees(180));
static_assert(pi2Rad/2 <= Degrees(180));
static_assert(three_sixty_deg/2 == Degrees(180));
static_assert(pi2Rad == Degrees(360));
static_assert((three_sixty_deg + 2) % 360 < 2.000000001);
static_assert(pi2Rad + 2 % 360 > 1.999999999);
static_assert(three_sixty_deg + 1 == 361);
static_assert(three_sixty_deg % 360 == 0);
static_assert(Degrees(pi2Rad) == 360);
static_assert(three_sixty_deg == 360);
static_assert(1_rad > Degrees(57.2957));
static_assert(1_rad < Degrees(57.2958));
static_assert(Meters(NAN).isNan());
static_assert(Meters(NAN) != 0);
static_assert(Milliradians(M_PI*2000) == Degrees(360));
//static_assert(MetersHAE(1) == MetersMSL(1)); // this shouldn't compile. Could use SFINAE to make it check.
static_assert(MetersHAE(1).toType<Meters>() == Meters(1));
static_assert(MetersMSL(1).toType<Meters>() == Meters(1));
static_assert(Meters(MetersHAE(3048).get()) == Feet(10000));
static_assert(KilometersMSL(1) == MetersMSL(1000));
static_assert(Knots(1.94384449244) == MetersPerSecond(1));
static_assert(std::abs(Meters(-4)) == 4);
static_assert(std::abs(Knots(4)) == 4);
static_assert(Feet(1) == Inches(12));
#undef ft
#undef pi2Rad
#undef three_sixty_deg