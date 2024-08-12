#pragma once
#include <ratio>
#include <functional>
#include <type_traits>
#include <cmath>
#include <iostream>

template <typename T, template<typename> class crtpType>
struct crtp
{
    constexpr T& underlying() { return static_cast<T&>(*this); }
    constexpr T const& underlying() const { return static_cast<T const&>(*this); }
};


template <typename T, typename Parameter, typename Ratio, template<typename> class... Skills>
class NamedTypeImpl;

template <typename T, typename Parameter, template<typename> class... Skills>
using NamedType = NamedTypeImpl<T, Parameter, std::ratio<1>, Skills...>;

template <typename T> struct ToDouble : crtp<T, ToDouble> {explicit constexpr operator double() const { return static_cast<double>(this->underlying().get());}};
template <typename T> struct ToInt : crtp<T, ToInt>       {explicit constexpr operator double() const { return static_cast<int>(this->underlying().get());}};
template <typename T> struct ToFloat : crtp<T, ToFloat>   {explicit constexpr operator double() const { return static_cast<float>(this->underlying().get());}};

template <typename T, typename Parameter, template<typename> class... Skills>
std::ostream& operator<<(std::ostream& os, NamedType<T, Parameter, Skills...> const& object)
{
    os << object.get();
    return os;
}


////////////////////////////////////////////////////////////////////////////////
template <typename T, typename Parameter, typename Ratio, template<typename> class... Skills>
class NamedTypeImpl : public Skills<NamedTypeImpl<T, Parameter, Ratio, Skills...>>...
{
public:
    // constructor
    constexpr NamedTypeImpl() = default;
    constexpr NamedTypeImpl(T const& value) : _value(value) {}
    constexpr NamedTypeImpl(const NamedTypeImpl& other): _value(other._value){}
//   template<typename T_ = T> constexpr NamedTypeImpl(T&& value, typename std::enable_if<!std::is_reference<T_>{}, std::nullptr_t>::type = nullptr) : _value(std::move(value)) {}

    // get
    constexpr T& get() { return _value; }
    constexpr T const& get() const {return _value; }

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
    constexpr NamedTypeImpl  operator-(NamedTypeImpl const& other) const {return _value +  NamedTypeImpl(other)._value;}
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
    struct SubRectCoords{};
    struct RadiansParameter{};
    struct PercentParameter{};
}

//our implementations
//using SubRectCoords = NamedType<int, StrongUnitParameters::SubRectCoords, ToInt>;
//using Meters = NamedType<double, StrongUnitParameters::MeterParameter, ToDouble>;
//using Kilometers = MultipleOf<Meters, std::kilo>;
//using Feet = MultipleOf<Meters, std::ratio<3048, 10000>>;
//constexpr Meters operator"" _m(unsigned long long value){return {(double)value};}
//constexpr Kilometers operator"" _km(unsigned long long value){return {(double)value};}
//constexpr Feet operator"" _ft(unsigned long long value){return {(double)value};}
//constexpr Meters operator"" _m(long double value){return {(double)value};}
//constexpr Kilometers operator"" _km(long double value){return {(double)value};}
//constexpr Feet operator"" _ft(long double value){return {(double)value};}
//
using Radians = NamedType<double, StrongUnitParameters::RadiansParameter, ToDouble>;
using Degrees = MultipleOf<Radians, std::ratio<31415926535897932, 1800000000000000000>>;
using Milliradians = MultipleOf<Radians, std::milli>;
using Perunum = NamedType<double, StrongUnitParameters::PercentParameter, ToDouble>;
using Perdeca = MultipleOf<Perunum, std::deci>;
using Percent = MultipleOf<Perunum, std::centi>;
using Permille = MultipleOf<Perunum, std::milli>;
//
constexpr Radians       operator"" _rad(unsigned long long value){return {(double)value};}
constexpr Degrees       operator"" _deg(unsigned long long value){return {(double)value};}
constexpr Milliradians  operator"" _mrad(unsigned long long value){return {(double)value};}
constexpr Radians       operator"" _rad(long double value){return {(double)value};}
constexpr Degrees       operator"" _deg(long double value){return {(double)value};}
constexpr Milliradians  operator"" _mrad(long double value){return {(double)value};}
constexpr Percent       operator"" _pct(unsigned long long value){return {(double)value};}
constexpr Percent       operator"" _pct(long double value){return {(double)value};}
//
//using MetersMSL = NamedType<double, StrongUnitParameters::MetersMSLParameter, ToDouble>; //does not implicitly convert to meters
//using KilometersMSL = MultipleOf<MetersMSL, std::kilo>;
//using MetersHAE = NamedType<double, StrongUnitParameters::MetersHAEParameter, ToDouble>; //does not implicitly convert to meters
//constexpr MetersMSL  operator"" _m_msl(unsigned long long value){return {(double)value};}
//constexpr MetersHAE  operator"" _m_hae(unsigned long long value){return {(double)value};}
//constexpr MetersHAE  operator"" _m_hae(long double value){return {(double)value};}
//constexpr MetersMSL  operator"" _m_msl(long double value){return {(double)value};}
//
//using MetersPerSecond = NamedType<double, StrongUnitParameters::MetersPerSecondParameter, ToDouble>;
//using Knots = MultipleOf<MetersPerSecond, std::ratio<1943844, 1000000>>;