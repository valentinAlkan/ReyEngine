#pragma once
#include "raylib.h"
#include "raymath.h"
#include <stdexcept>
#include <cfloat>
#include <string>
#include <array>
#include <iostream>
#include "StringTools.h"
#include "Logger.h"
#include "FileSystem.h"
#include "Property.h"
#include "StrongUnits.h"
#ifdef linux
#include <limits.h>
#endif

#define R_FLOAT float //float or double?
#define STOF std::stof
#define NOT_IMPLEMENTED throw std::runtime_error("Not implemented!")

namespace ReyEngine {
   static constexpr long long MaxInt = INT_MAX;
   static constexpr long long MinInt = INT_MIN;
   static constexpr R_FLOAT MaxFloat = FLT_MAX;
   static constexpr R_FLOAT MinFloat = FLT_MIN;

    namespace Math {
        template <typename T> T min(T a, T b){return a <= b ? a : b;}
        template <typename T> T max(T a, T b){return a >= b ? a : b;}
    }

   template <typename T> struct Size;
   template <typename T> struct Pos;
   template <typename T>
   struct Vec {
      constexpr explicit Vec(size_t size): size(size){}
      inline static std::vector<T> fromString(size_t size, const std::string& s) {
         std::string sanitized;
         for (const auto &c: s) {
            if (::isdigit(c) || c == '-' || c == ',' || c == '.') {
               sanitized += c;
            }
         }
         std::vector<T> retval;
         auto split = string_tools::split(sanitized, ",");
         if (split.size() != size) {
            return retval;
         }

         for (size_t i = 0; i < size; i++) {
            retval.push_back(std::stod(split[i]));
         }
         return retval;
      }
      [[nodiscard]] inline std::string toString() const {
         auto elements = getElements();
         std::string retval = "{";
         for (const auto& element : elements){
            retval += std::to_string(element);
            retval += ", ";
         }
         //remeove trailing comma
         retval.pop_back();
         retval.pop_back();
         retval += "}";
         return retval;
      }
      [[nodiscard]] virtual std::vector<T> getElements() const = 0;
      const size_t size;
   };


   template <typename T>
   struct Vec2 : protected Vec<T> {
      using Vec<T>::toString;
      constexpr inline Vec2(): Vec<T>(2), x(0), y(0){}
      constexpr inline Vec2(const T& x, const T& y) : Vec<T>(2), x(x), y(y){}
      constexpr inline Vec2(const Vector2& v)     : Vec<T>(2), x((T)v.x), y((T)v.y){}
      template <typename R>
      constexpr inline Vec2(const Vec2<R>& v): Vec<T>(2), x((T)v.x), y((T)v.y){}
      inline explicit operator bool() const {return x || y;}
      inline Vec2 operator+(const Vec2& rhs) const {Vec2<T> val = *this; val.x += rhs.x; val.y += rhs.y; return val;}
      inline Vec2 operator-(const Vec2& rhs) const {Vec2<T> val = *this; val.x -= rhs.x; val.y -= rhs.y; return val;}
      inline Vec2& operator+=(const Vec2& rhs){x += rhs.x; y += rhs.y; return *this;}
      inline Vec2& operator-=(const Vec2& rhs){x -= rhs.x; y -= rhs.y; return *this;}
      inline Vec2 operator*(double rhs) const {Vec2 retval(*this); retval.x *= rhs; retval.y *= rhs; return retval;}
//      inline Vec2& operator*=(const Vec2& rhs){x *= rhs.x; y *= rhs.y; return *this;}
      inline Vec2& operator*=(double rhs){x *= rhs; y *= rhs; return *this;}
      inline Vec2<R_FLOAT> operator/(double rhs) const {Vec2<R_FLOAT> retval(*this); retval.x /= rhs; retval.y /= rhs; return retval;}
      inline Vec2& operator/=(const Vec2& rhs){x /= rhs.x; y /= rhs.y; return *this;}
      inline Vec2& operator=(const Vec2& rhs){x = rhs.x; y=rhs.y; return *this;}
      inline bool operator==(const Vec2& rhs) const {return x==rhs.x && y==rhs.y;}
      inline bool operator!=(const Vec2& rhs) const {return x!=rhs.x || y!=rhs.y;}
      inline Vec2 operator-() const {return {-x, -y};}
      inline void operator=(Size<T>&) = delete;
      inline void operator=(Pos<T>&) = delete;
      inline explicit operator Vector2() const {return {(float)x,(float)y};}
      inline Vec2 midpoint() const {return {x/2, y / 2};}
      inline Vec2 min(const Vec2& other) const {Vec2 r; r.x = Math::min(Vec2::x, other.x); r.y = Math::min(Vec2::y, other.y); return r;}
      inline Vec2 max(const Vec2& other) const {Vec2 r; r.x = Math::max(Vec2::x, other.x); r.y = Math::max(Vec2::y, other.y); return r;}
      inline Perunum pct(double input) const {return (input-x)/(y - x);} //given an input value, what percentage of the range is it from 0 to 1?
      inline double lerp(Perunum lerpVal) const {return lerpVal.get() * (y - x) + x;} //given a value from 0 to 1, what is the value of the range that corresponds to it?
      inline Vec2 lerp(Vec2 otherPoint, double xprm) const {return {xprm, y + (((xprm - x) * (otherPoint.y - y)) / (otherPoint.x - x))};}
      inline T clamp(T value) const {if (value < x) return x; if (value > y) return y; return value;}
      inline Vec2 clamp(Vec2 clampA, Vec2 clampB) const {
         Vec2 retval = {x, y};
         if (x < clampA.x) retval.x = clampA.x;
         if (x > clampB.x) retval.x = clampB.x;
         if (y < clampA.x) retval.y = clampA.y;
         if (y > clampB.y) retval.y = clampB.y;
         return retval;
      }
      inline double length(){return std::sqrt(x * x + y * y);}
      inline Vec2<T> normalize(){double len = length();return {(T)(x / len), (T)(y / len)};}
      inline static std::vector<T> fromString(const std::string& s){return Vec<T>::fromString(2, s);};
      friend std::ostream& operator<<(std::ostream& os, Vec2<T> v) {os << v.toString(); return os;}
      friend Vector2& operator+=(Vector2& in, Vec2<T> add) {in.x += add.x; in.y += add.y; return in;}
      T x;
      T y;
      [[nodiscard]] inline std::vector<T> getElements() const override {return {x,y};}
   };

   template <typename T>
   struct Vec3 : protected Vec<T> {
      using Vec<T>::toString;
      inline Vec3(): Vec<T>(3), x(0), y(0), z(0){}
      inline operator Vector3() const {return {(float)x,(float)y,(float)z};}
      inline Vec3(const T& _x, const T& y, const T& _z) : Vec<T>(3), x(_x), y(y),z(_z) {}
      inline explicit Vec3(const Vector3& v)     : Vec<T>(3), x((T)v.x), y((T)v.y), z((T)v.z){}
      inline Vec3(const Vec3& v): Vec<T>(3), x((T)v.x), y((T)v.y), z((T)v.z){}
      inline double length(){return std::sqrt(x * x + y * y + z * z);}
      inline Vec3 normalize(){
          auto len = length();
          if (len != 0){return {x/len, y/len, z/len};}
          return {};
      }
      inline Vec3& operator=(const Vec3& rhs){x = rhs.x; y=rhs.y; z=rhs.z; return *this;}
      inline Vec3& operator-(){x = -x; y =-y; z = -z; return *this;}
      inline Vec3 operator-(const Vec3& rhs) const {Vec3 retval; retval.x=x-rhs.x; retval.y=y-rhs.y; retval.z=z-rhs.z; return retval;}
      inline Vec3 operator+(const Vec3& rhs) const {Vec3 retval; retval.x=x+rhs.x; retval.y=y+rhs.y; retval.z=z+rhs.z; return retval;}
      inline Vec3& operator-=(const Vec3& rhs){x-=rhs.x; y-=rhs.y; z-=rhs.z; return *this;}
      inline Vec3& operator+=(const Vec3& rhs){x+=rhs.x; y+=rhs.y; z+=rhs.z; return *this;}
      inline static std::vector<T> fromString(const std::string& s){return Vec<T>::fromString(3, s);};
      [[nodiscard]] inline std::vector<T> getElements() const override {return {x,y,z};}
      inline std::optional<T> normalize() const {
         const auto& len = magnitude();
         if (len == T(0)) return {};
         return {x / len, y / len, z / len};
      }
      inline T magnitude() const {return std::sqrt(x * x + y * y + z * z);}
      inline T dot(const Vec3& rhs) const {return x * rhs.x + y * rhs.y + z * rhs.z;}
      inline static T dot(const Vec3& a, const Vec3& b){return a.dot(b);}
      inline Vec3<T> cross(const Vec3& rhs) const {return {y * rhs.z - z * rhs.y,z * rhs.x - x * rhs.z,x * rhs.y - y * rhs.x};}
      inline static Vec3<T> cross(const Vec3& a, const Vec3& b){return a.cross(b);}
      friend std::ostream& operator<<(std::ostream& os, const Vec3& v) {os << v.toString(); return os;}
      T x;
      T y;
      T z;
   };

    template <typename T>
    struct Vec4 : protected Vec<T> {
        inline Vec4(): Vec<T>(4), w(0), x(0), y(0), z(0){}
        inline Vec4(const std::vector<T>& stdvec): Vec<T>(4){
            if (stdvec.size() != 4) throw std::runtime_error("Invalid element count for Vec4! Expected 4, got " + stdvec.size());
            w = stdvec[0];
            x = stdvec[1];
            y = stdvec[2];
            z = stdvec[3];
        }
        inline Vec4(const T& _w, const T& _x, const T& y, const T& _z) : Vec<T>(4), w(_w), x(_x), y(y),z(_z) {}
        inline explicit Vec4(const Vector4& v) : Vec<T>(4), w((T)v.w), x((T)v.x), y((T)v.y), z((T)v.z){}
        template <typename R>
        inline Vec4(const Vec4<R>& v)        : Vec<T>(4), w((T)v.w), x((T)v.x), y((T)v.y), z((T)v.z){}
        inline Vec4& operator=(const Vec4& rhs){w = rhs.w, x = rhs.x; y=rhs.y; z=rhs.z; return *this;}
        inline Vec4& operator-(){w = -w; x = -x; y =-y; z = -z; return *this;}
        inline static std::vector<T> fromString(const std::string& s){return Vec<T>::fromString(4, s);};
        [[nodiscard]] inline std::vector<T> getElements() const override {return {w,x,y,z};}
        friend std::ostream& operator<<(std::ostream& os, Vec4 v) {os << v.toString(); return os;}
        using ReyEngine::Vec<T>::toString;
        T w;
        T x;
        T y;
        T z;
    };

   template <typename T>
   class Range : private Vec3<T> {
   public:
      //Vec3, but x represents min, y represents max, and z represents value; Can lerp and set pct.
      //When creating from Vec3, vec3 shall be considered x=min, y=max, z=default value.
      //Does not enforce limits (ie value is free to be more or less than min or max.
      inline Range(): Vec3<T>::Vec3(){}
      inline Range(const T& min, const T& max, const T& defaultValue) : Vec3<T>::Vec3(min, max, defaultValue){}
      inline explicit Range(const Vector3& v)     : Vec3<T>::Vec3(v){}
      template <typename R>
      inline explicit Range(const Vec3<R>& v)   : Vec3<T>::Vec3(v){}
      inline Perunum getpct(){return getRange().pct(getValue());};
      inline void setLerp(Perunum pct){setValue(getRange().lerp(pct));}
      inline T getMin() const {return Vec3<T>::x;}
      inline T getMax() const {return Vec3<T>::y;}
      inline T getValue() const {return Vec3<T>::z;}
      inline T setMin(T value){return Vec3<T>::x = value;}
      inline T setMax(T value){return Vec3<T>::y = value;}
      inline T setValue(T value){return Vec3<T>::z = value;}
      inline Vec2<T> getRange(){return Vec2<T>(Vec3<T>::x, Vec3<T>::y);}
      inline void setRange(Vec2<T> newRange){ setMin(newRange.x); setMax(newRange.y);}
      inline void setRange(T min, T max){ setMin(min); setMax(max);}
      inline static void fromString(const std::string& s){return Vec3<T>::fromString();};
      inline std::vector<T> getElements() const override {return Vec3<T>::getElements();}
      friend std::ostream& operator<<(std::ostream& os, Range<T> r) {os << r.toString(); return os;}
   };

   template <typename T>
   struct Line {
      template <typename _t>
      constexpr Line(const Line<_t>& other): Line(other.a, other.b){}
      constexpr Line(): a(0,0), b(0,0){}
      constexpr Line(Pos<T> a, Pos<T> b): a(a), b(b){}
      constexpr Line(const T x1, const T y1, const T x2, const T y2): Line({x1, y1}, {x2, y2}){}
      constexpr Pos<T> midpoint() const {return {a.x/2+b.x/2, a.y/2+b.y/2};}
      constexpr Pos<T> lerp(double xprm) const {return a.lerp(b, xprm);}
      constexpr double distance() const {return a.distanceTo(b);}
      inline std::string toString() const {return "{" + a.toString() + ", " + b.toString() + "}";}
      constexpr Line& operator+=(const Pos<T>& pos){a += pos; b += pos; return *this;}
      constexpr Line operator+(const Pos<T>& pos) const {Line<T> l(*this); l.a += pos; l.b += pos; return l;}
      //Find the angle from horizontal between points and a b
      inline Radians angle() const {
          auto dx = static_cast<R_FLOAT>(b.x - a.x);
          auto dy = static_cast<R_FLOAT>(b.y - a.y);
          return atan2(dy, dx);
      }
       //rotate the line around A by r radians
      inline Line rotate(Pos<T> basis, Radians r) const {return {a.rotatePoint(basis, r), b.rotatePoint(basis, r)};}
      inline Line pctOf(Percent pct) const {
          // Calculate the direction vector from a to b
          auto direction = b - a;
          // Scale the direction vector by the percentage
          auto extensionVector = direction *Perunum(pct).get();
          auto newB = a + Pos(extensionVector.x, extensionVector.y);
          return {a, newB};
      }
      //project a line a fixed amount from the start point
      inline Line project(double amount) const {return {a, a.project(b, amount)};}
      inline T slope(){
         if (a.x == b.x) throw std::runtime_error("Undefined slope (vertical line)");
         return (b.y - a.y) / (b.x - a.x);
      }
      inline std::pair<Line, Line> normals(){
         auto dx = b.x - a.x;
         auto dy = b.y - a.y;
         return {{-dy, dx}, {dy, -dx}};
      }
      friend std::ostream& operator<<(std::ostream& os, Line r) {os << r.toString(); return os;}
      Pos<T> a;
      Pos<T> b;
   };

   template <typename T=R_FLOAT>
   struct Pos : public Vec2<T>{
      inline Pos(): Vec2<T>(){}
      inline Pos(const T& x, const T& y) : Vec2<T>(x, y){}
      inline Pos(const Vector2& v) : Vec2<T>(v){}
      template <typename R>
      inline Pos(const Vec2<R>& v) : Vec2<T>(v){}
      inline void operator=(Size<T>&) = delete;
      template <typename R>
      inline Pos& operator=(const Pos<R>& other){Vec2<T>::x = other.x; Vec2<T>::y=other.y; return *this;}
      inline Pos operator+(const Pos& rhs) const {auto val = *this; val.x += rhs.x; val.y += rhs.y; return val;}
      inline Pos operator-(const Pos& rhs) const {auto val = *this; val.x -= rhs.x; val.y -= rhs.y; return val;}
      inline Pos operator-() const {return {-Vec2<T>::x, -Vec2<T>::y};}
      inline Pos& operator+=(const Pos& rhs){this->x += rhs.x; this->y += rhs.y; return *this;}
      inline Pos& operator-=(const Pos& rhs){this->x -= rhs.x; this->y -= rhs.y; return *this;}
      inline bool operator!=(const Pos& rhs){return this->x != rhs.x || this->y != rhs.y;}
      inline operator std::string() const {return Vec2<T>::toString();}
      inline void operator=(const Size<T>&) = delete;
      inline Pos clamp(Pos clampA, Pos clampB) const { return Pos(Vec2<T>::clamp(clampA, clampB));}
//      inline Pos& operator=(const Vec2<T>& other){Pos::x = other.x; Pos::y = other.y; return *this;}
//      Rotate around a basis point
      inline Pos rotatePoint(const Pos<int>& basis, Radians r) const {
           double radians = r.get();
           // Translate point to origin
           double xTranslated = Pos::x - basis.x;
           double yTranslated = Pos::y - basis.y;
           // Apply rotation and translate back
           Pos<int> p_rotated;
           p_rotated.x = static_cast<int>(xTranslated * cos(radians) - yTranslated * sin(radians) + basis.x);
           p_rotated.y = static_cast<int>(xTranslated * sin(radians) + yTranslated * cos(radians) + basis.y);
           return p_rotated;
       }
      // Function to project a point distance d from point a along the line ab
      inline Pos project(const Pos& b, double d) const {
          // Calculate the direction vector from a to b
          Pos direction = b - *this;
          // Normalize the direction vector
          Pos unitDirection = direction.normalize();
          // Scale the normalized vector by distance d
          Pos scaledDirection = unitDirection * d;
          // Calculate the new point by adding the scaled direction to point a
          Pos projectedPoint = *this + Pos(scaledDirection.x, scaledDirection.y);
          return projectedPoint;
      }
      double distanceTo(const Pos& other) const {
         auto diff = *this - other;
         return std::sqrt(diff.x * diff.x + diff.y * diff.y);
      }
      inline std::string toString() const {return Vec2<T>::toString();}
   };

   template <typename T>
   struct PosProperty : public Property<Pos<T>>{
      using Property<Pos<T>>::operator=;
      PosProperty(const std::string& instanceName,  Pos<T>&& defaultvalue={})
      : Property<Pos<T>>(instanceName, PropertyTypes::Pos, std::move(defaultvalue))
      {}
      std::string toString() const override {return "";}
      Pos<T> fromString(const std::string& str) override {return {};}
   };

   template <typename T>
   struct Size : public Vec2<T>{
      constexpr inline Size(): Vec2<T>(){}
      constexpr inline Size(const T& x, const T& y) : Vec2<T>(x, y){}
      explicit inline Size(const T edge): Size(edge, edge){}
      constexpr inline Size(const Vector2& v)     : Vec2<T>(v.x,v.y){}
      template <typename R>
      constexpr inline Size(const Vec2<R>& v)   : Vec2<T>(v.x,v.y){}
      inline Size(const Size<T>& v) : Vec2<T>(v){}
      inline void operator=(Pos<T>&) = delete;
      inline bool operator==(const Size<T>& rhs) const {return Size::x==rhs.x && Size::y==rhs.y;}
      inline bool operator!=(const Size<T>& rhs) const {return Size::x!=rhs.x || Size::y!=rhs.y;}
      inline Size operator+(const Size& rhs) const {auto val = *this; val.x += rhs.x; val.y += rhs.y; return val;}
      inline Size operator-(const Size& rhs) const {auto val = *this; val.x -= rhs.x; val.y -= rhs.y; return val;}
      inline Size& operator+=(const Size& rhs){this->x += rhs.x; this->y += rhs.y; return *this;}
      inline Size& operator-=(const Size& rhs){this->x -= rhs.x; this->y -= rhs.y; return *this;}
      inline operator std::string() const {return Vec2<T>::toString();}
   };

   template <typename T>
   struct SizeProperty : public Property<Size<T>>{
      using Property<Size<T>>::operator=;
      SizeProperty(const std::string& instanceName,  Size<T>&& defaultvalue={})
      : Property<Size<T>>(instanceName, PropertyTypes::Size, std::move(defaultvalue))
      {}
      std::string toString() const override {return "";}
      Size<T> fromString(const std::string& str) override {return {};}
   };

   struct Circle;
   template <typename T>
   struct Rect {
      using SubRectCoords = NamedType<Vec2<int>, StrongUnitParameters::SubRectCoords>;
      enum class Corner{TOP_LEFT=1, TOP_RIGHT=2, BOTTOM_RIGHT=4, BOTTOM_LEFT=8};
      constexpr inline Rect(): x(0), y(0), width(0), height(0){}
      constexpr inline Rect(const T x, const T y, const T width, const T height) : x(x), y(y), width(width), height(height){}
      constexpr inline explicit Rect(const Rectangle& r): x((T)r.x), y((T)r.y), width((T)r.width), height((T)r.height){}
      template <typename R>
      constexpr inline Rect(const Rect<R>& r): x((T)r.x), y((T)r.y), width((T)r.width), height((T)r.height){}
      inline explicit Rect(const Vec2<T>&) = delete;
      constexpr inline explicit Rect(const Pos<T>& v): x((T)v.x), y((T)v.y), width(0), height(0){}
      constexpr inline explicit Rect(const Size<T>& v): x(0), y(0), width((T)v.x), height((T)v.y){}
      constexpr inline operator bool(){return x || y || width || height;}
      constexpr inline Rect(const Pos<T>& pos, const Size<T>& size): x((T)pos.x), y((T)pos.y), width((T)size.x), height((T)size.y){}
      constexpr inline bool operator==(const Rect<T>& rhs) const {return rhs.x == x && rhs.y == y && rhs.width == width && rhs.height == height;}
      constexpr inline bool operator!=(const Rect<T>& rhs) const {return !(*this == rhs);}
      constexpr inline Rect operator+(const Pos<T>& rhs) const {Rect<T> val = *this; val.x += rhs.x; val.y += rhs.y; return val;}
      constexpr inline Rect operator-(const Pos<T>& rhs) const {Rect<T> val = *this; val.x -= rhs.x; val.y -= rhs.y; return val;}
      constexpr inline Rect& operator+=(const Pos<T>& rhs){x += rhs.x; y += rhs.y; return *this;}
      constexpr inline Rect& operator-=(const Pos<T>& rhs){x -= rhs.x; y -= rhs.y; return *this;}
      constexpr inline Rect& operator*=(const Pos<T>& rhs){x *= rhs.x; y *= rhs.y; return *this;}
      constexpr inline Rect& operator/=(const Pos<T>& rhs){x /= rhs.x; y /= rhs.y; return *this;}
      constexpr inline Rect operator+(const Size<T>& rhs) const {Rect<T> val = *this; val.width += rhs.x; val.height += rhs.y; return val;}
      constexpr inline Rect operator-(const Size<T>& rhs) const {Rect<T> val = *this; val.width -= rhs.x; val.height -= rhs.y; return val;}
      constexpr inline Rect& operator+=(const Size<T>& rhs){width += rhs.width; height += rhs.height; return *this;}
      constexpr inline Rect& operator-=(const Size<T>& rhs){width -= rhs.width; height -= rhs.height; return *this;}
      constexpr inline Rect& operator*=(const Size<T>& rhs){width *= rhs.width; height *= rhs.height; return *this;}
      constexpr inline Rect& operator/=(const Size<T>& rhs){width /= rhs.width; height /= rhs.height; return *this;}
      constexpr inline Rect operator+(const Rect<T>& rhs) const {Rect<T> val = *this; val.x += rhs.x; val.y += rhs.y; val.width += rhs.width; val.height += rhs.height; return val;}
      constexpr inline Rect operator-(const Rect<T>& rhs) const {Rect<T> val = *this; val.x -= rhs.x; val.y -= rhs.y; val.width -= rhs.width; val.height -= rhs.height; return val;}
      constexpr inline Rect& operator+=(const Rect<T>& rhs){x += rhs.x; y += rhs.y; width += rhs.width; height += rhs.height; return *this;}
      constexpr inline Rect& operator-=(const Rect<T>& rhs){x -= rhs.x; y -= rhs.y; width -= rhs.width; height -= rhs.height; return *this;}
      constexpr inline Rect& operator*=(const Rect<T>& rhs){x *= rhs.x; y *= rhs.y; width *= rhs.width; height *= rhs.height; return *this;}
      constexpr inline Rect& operator/=(const Rect<T>& rhs){x /= rhs.x; y /= rhs.y; width /= rhs.width; height /= rhs.height; return *this;}
      constexpr inline operator Rectangle() {return {x,y,width,height};}
      constexpr inline Rect embiggen(T amt) const {return *this + Rect<T>(-amt, -amt, 2*amt, 2*amt);} //shrink/expand borders evenly
      constexpr inline Rect emtallen(T amt) const {return  *this + Rect<T>(0, -amt, 0, 2*amt);}//embiggen tallness
      constexpr inline Rect emwiden(T amt) const {return  *this + Rect<T>(-amt, 0, 2*amt, 0);}//embiggen wideness
      constexpr inline Rect chopTop(T amt) const {auto retval = *this; retval.y+= amt; retval.height-=amt; return retval;} //remove the topmost amt of the rectangle and return the remainder (moves y, cuts height)
      constexpr inline Rect chopBottom(T amt) const {auto retval = *this; retval.height-=amt; return retval;} //remove the bottommost amt of the rectangle and return the remainder (cuts height)
      constexpr inline Rect chopRight(T amt) const {auto retval = *this; retval.width-=amt; return retval;} //remove the rightmost amt of the rectangle and return the remainder (cuts width)
      constexpr inline Rect chopLeft(T amt) const {auto retval = *this; retval.x+=amt; retval.width-=amt; return retval;} //remove the leftmost amt of the rectangle and return the remainder (moves x, cuts width)
      constexpr inline bool isInside(const Vec2<T>& point) const {return (point.x >= x && point.x <= x + width) && (point.y >= y && point.y <= y + height);}
      constexpr inline bool isInside(const Rect& other) const {return other.x+other.width <= x+width && other.x >= x && other.y >= y && other.y+other.height <= y+height;}
      constexpr inline Pos<T> topLeft() const {return {x, y};}
      constexpr inline Pos<T> topRight() const {return {x+width, y};}
      constexpr inline Pos<T> bottomRight() const {return {x+width, y+height};}
      constexpr inline Pos<T> bottomLeft() const {return {x, y+height};}
      constexpr inline Line<T> leftSide() const {return {topLeft(), bottomLeft()};}
      constexpr inline Line<T> rightSide() const {return {topRight(), bottomRight()};}
      constexpr inline Line<T> top() const {return {topLeft(), topRight()};}
      constexpr inline Line<T> bottom() const {return {bottomLeft(), bottomRight()};}
      constexpr inline bool collides(const Rect& other) const {
         return ((x < (other.x + other.width) && (x + width) > other.x) &&
             (y < (other.y + other.height) && (y + height) > other.y));
      }
      constexpr inline int getCollisionType(const Rect& other) const {
         int pointCount = 0;
         if (isInside(other.topLeft())) pointCount++;
         if (isInside(other.topRight())) pointCount++;
         if (isInside(other.bottomRight())) pointCount++;
         if (pointCount == 3) return 3;
         if (isInside(other.bottomLeft())) pointCount++;
         return pointCount;
      }
      constexpr inline Rect getOverlap(const Rect& other) const {
         //this is pretty naive, but whatever
         //tag the coordinates
         auto& xl = x < other.x ? x : other.x;
         auto& xr = x > other.x ? x : other.x;
         auto& yt = y < other.y ? y : other.y;
         auto& yb = y > other.y ? y : other.y;
         auto& xlw = x < other.x ? width : other.width;
         auto& xrw = x > other.x ? width : other.width;
         auto& yth = y < other.y ? height : other.height;
         auto& ybh = y > other.y ? height : other.height;

         //a primary collision is when other bisects us (in a 2 point collision)
         int collisionType = getCollisionType(other);
         bool isSecondaryCollision = false;
         if (!collisionType){
            collisionType = other.getCollisionType(*this);
            isSecondaryCollision = true;
         }
         auto& primaryRect = isSecondaryCollision ? other : *this;
         auto& secondaryRect = isSecondaryCollision ? *this : other;
         switch (collisionType){
            case 0:
               //potentially a collision or not
               if (xr > xl+xlw || yb > yt+yth) return {}; //no collision
               //cross collision
               return {xr, yb, xrw, ybh};
            case 2:
               //2 point collision
               //determine if its a full width/height collision and should therefore take 1 point form
               if (primaryRect.x != secondaryRect.x && primaryRect.y != secondaryRect.y) {
                  //determine if its horizontal or vertical form
                  auto xprime = secondaryRect.x > primaryRect.x;
                  auto yprime = secondaryRect.y > primaryRect.y;
                  bool horizontalForm = true;
                  if (xprime && yprime){
                     //need to distinguish between forms 1 and 2 which both have the top left prime point inside the rect
                     if (isInside(secondaryRect.bottomLeft())){
                        //form 2
                        horizontalForm = false;
                     }
                  } else if ((!xprime && yprime) || (!xprime && !yprime)){
                     horizontalForm = false;
                  }
                  if (horizontalForm) return {xr, yb, xrw, yt+yth-yb};
                  return {xr, yb, xl+xlw-xr, ybh};
               }
               //otherwise fall through to 1 point form
            case 1:
               //corner collision, full width/height 2 point collisions
               return {xr, yb, xl+xlw-xr, yt+yth-yb};
            default:
               //fully inside, 3 or 4 point collisions (we won't actually get 4 point but its handled here just in case
               return secondaryRect;
         }
      }
      [[nodiscard]] constexpr inline Vec2<T> center() const {return {x+width/2, y+height/2};}
      constexpr inline void setCenter(const Vec2<T>& center) {x = center.x-width/2; y= center.y - height / 2;}
      [[nodiscard]] inline std::string toString() const {
         return "{" + std::to_string(x) + ", " + std::to_string(y) + ", " +
         std::to_string(width) + ", " + std::to_string(height) + "}";
      }
      inline static ReyEngine::Rect<T> fromString(const std::string& s){
         std::string sanitized;
         for (const auto& c : s){
            if (::isdigit(c) || c == '-' || c==',' || c=='.'){
               sanitized += c;
            }
         }
         auto split = string_tools::split(sanitized, ",");
         if (split.size() != 4){
            return ReyEngine::Rect<T>();
         }

         ReyEngine::Rect<T> retval;
         retval.x = std::stoi(split[0]);
         retval.y = std::stoi(split[1]);
         retval.width = std::stoi(split[2]);
         retval.height = std::stoi(split[3]);
         return retval;
      }
      friend std::ostream& operator<<(std::ostream& os, const Rect<T>& r){
         os << r.toString();
         return os;
      }
      [[nodiscard]] constexpr inline const Pos<T> pos() const {return {x, y};}
      [[nodiscard]] constexpr inline const Size<T> size() const {return {width, height};}
      [[nodiscard]] constexpr inline const Rect<T> toSizeRect() const {return {0,0,width, height};}
      constexpr inline void setSize(const ReyEngine::Size<T>& size){width = size.x; height = size.y;}
      constexpr inline void setPos(const ReyEngine::Pos<T>& pos){x = pos.x; y = pos.y;}

      //return the smallest rect that contains both rects a and b
      constexpr inline Rect getBoundingRect(const Rect& other) const {
         // Find the bottom-right corner coordinates for both rectangles
         int _right1 = x + width;
         int _bottom1 = y + height;
         int _right2 = other.x + other.width;
         int _bottom2 = other.y + other.height;
         // Find the top-left corner coordinates of the bounding rectangle
         int _left = std::min(x, other.x);
         int _top = std::min(y, other.y);
         // Find the bottom-right corner coordinates of the bounding rectangle
         int _right = std::max(_right1, _right2);
         int _bottom = std::max(_bottom1, _bottom2);
         // Ensure width and height are non-negative (handles cases where rectangles don't intersect)
         int _width = std::max(0, _right - _left);
         int _height = std::max(0, _bottom - _top);
         // Return a new Rect with top-left corner and dimensions
         return Rect(_left, _top, _width, _height);
      }
      constexpr inline Rect getBoundingRect(const Rect& a, const Rect& b) const {
         return a.getBoundingRect(b);
      }

      //returns the 'index' of a subrect, as if it were read left-to-right, top-to-bottom
      constexpr inline int getSubRectIndex(const Size<R_FLOAT>& size, const Pos<R_FLOAT>& pos) const {
         auto coord = getSubRectCoord(size, pos);
         auto columnCount = width / size.x;
         return coord.get().y * columnCount + coord.get().x;
      }

      //Get the sub-rectangle (of size Size) that contains pos Pos. Think tilemaps.
      constexpr inline Rect getSubRectAtPos(const Size<R_FLOAT>& size, const Pos<R_FLOAT>& pos) const {
         auto subx = pos.x / size.x;
         auto suby = pos.y / size.y;
         return Rect(subx * size.x, suby*size.y, size.x, size.y);
      }

       //Get the sub-rectangle (of size Size) at SubRectCoords coords.
       constexpr inline Rect getSubRectAtCoords(const Size<R_FLOAT>& size, const SubRectCoords& coords) const {
           return Rect(coords.get().x * size.x, coords.get().y*size.y, size.x, size.y);
       }

      //returns the coordinates of the above subrect in grid-form (ie the 3rd subrect from the left would be {3,0}
      constexpr inline SubRectCoords getSubRectCoord(const Size<R_FLOAT>& size, const Pos<R_FLOAT>& pos) const {
         //divide by 0?
         auto subx = pos.x / size.x;
         auto suby = pos.y / size.y;
         return SubRectCoords({subx, suby});
      }

      //get an actual subrect given a subrect size and an index
      constexpr inline Rect getSubRect(const Size<R_FLOAT>& size, int index) const {
         int columnCount = width / size.x;
         int coordY = index / columnCount;
         int coordX = index % columnCount;
         R_FLOAT posX = (float)coordX * size.x;
         R_FLOAT posY = (float)coordY * size.y;
         return Rect(posX, posY, size.x, size.y);
      }
      //get the rectangle that contains the subrects at start and stop indices (as topleft and bottom right respectively)
      constexpr inline Rect getSubRect(const Size<R_FLOAT>& size, int indexStart, int indexStop) const {
         auto a = getSubRect(size, indexStart);
         auto b = getSubRect(size, indexStop);
         return getBoundingRect(a,b);
      }
      Circle circumscribe();
      Circle inscribe();

      constexpr inline void clear(){x=0,y=0,width=0;height=0;}
      T x;
      T y;
      T width;
      T height;
   };

   struct Circle{
      inline Circle(const Pos<R_FLOAT>& center, double radius): center(center), radius(radius){}
      inline Circle(const Circle& rhs): center(rhs.center), radius(rhs.radius){}
      /// create the circle that comprises the three points
      static inline std::optional<Circle> fromPoints(const Pos<R_FLOAT>& a, const Pos<R_FLOAT>& b, const Pos<R_FLOAT>& c){
         // Convert input points to doubles for precise calculation
         double x1 = a.x, y1 = a.y;
         double x2 = b.x, y2 = b.y;
         double x3 = c.x, y3 = c.y;

         // Calculate the perpendicular bisector of two chords
         double ux = 2 * (x2 - x1);
         double uy = 2 * (y2 - y1);
         double vx = 2 * (x3 - x1);
         double vy = 2 * (y3 - y1);
         double u = (x2*x2 - x1*x1 + y2*y2 - y1*y1);
         double v = (x3*x3 - x1*x1 + y3*y3 - y1*y1);

         // Calculate determinant
         double det = ux * vy - uy * vx;

         if (std::abs(det) < 1e-6) {
            //colinear points. not a circle.
            return {};
         }
         // Calculate center coordinates
         R_FLOAT cx = (u * vy - v * uy) / det;
         R_FLOAT cy = (v * ux - u * vx) / det;
         return Circle({cx, cy}, std::sqrt((cx - x1)*(cx - x1) + (cy - y1)*(cy - y1)));
      }
      /// Return a point on the circle that corresponds to the given angular offset from right-handed horizontal
      /// \param r
      /// \return
      inline Pos<R_FLOAT> getPoint(Radians r) const {return {center.x + radius * std::cos(r.get()), center.y + radius * std::sin(r.get())};}
      /// Return the angular offset from the right-handed horizontal that corresponds to the given point.
      ///
      /// \param pos: A point along a normal
      /// \return
      inline Radians getRadians(const Pos<R_FLOAT>& pos) const {
         double dx = pos.x - center.x;
         double dy = pos.y - center.y;
         double angle = std::atan2(dy, dx);
         // Ensure the angle is in the range [0, 2π)
         if (angle < 0) {
            angle += 2 * M_PI;
         }
         return Radians(angle);
      }
      /// Returns the point that intersects with the circle and lies along the normal formed by the point and the circle.
      /// \return
      inline Pos<R_FLOAT> getTangentPoint(const Pos<R_FLOAT>& pos) const {return getPoint(getRadians(pos));}
      /// Returns a line that is tangent to the circle at the given normal point.
      /// \param pos: a point lying on a line that is normal to the circle.
      /// \param length: the length of the tangent
      /// \return: a tangent line that intersects the given normal
      inline Line<R_FLOAT> getTangentLine(const Pos<R_FLOAT>& pos, double length) const {
         auto point = getTangentPoint(pos);
         // Calculate the vector from center to tangent point
         double dx = point.x - center.x;
         double dy = point.y - center.y;
         // Calculate the perpendicular vector (rotate by 90 degrees)
         double perpX = -dy;
         double perpY = dx;
         // Normalize the perpendicular vector
         double magnitude = std::sqrt(perpX*perpX + perpY*perpY);
         perpX /= magnitude;
         perpY /= magnitude;
         // Calculate the start and end points of the tangent line
         double halfLength = length / 2.0;
         Pos start(point.x - perpX * halfLength, point.y - perpY * halfLength);
         Pos end(point.x + perpX * halfLength, point.y + perpY * halfLength);
         return Line<R_FLOAT>(start, end);
      }
      bool collides(const Circle& other) const {
         return (center - other.center).length() < radius + other.radius;
      }
      bool collides(const Rect<R_FLOAT>& rect) const {
         // Find the closest point on the rectangle to the circle's center
         R_FLOAT closestX = std::max(rect.x, std::min(center.x, rect.x + rect.width));
         R_FLOAT closestY = std::max(rect.y, std::min(center.y, rect.y + rect.height));

         // Calculate the distance between the circle's center and the closest point
         R_FLOAT distanceX = center.x - closestX;
         R_FLOAT distanceY = center.y - closestY;

         // Check if the distance is less than or equal to the circle's radius
         return (distanceX * distanceX + distanceY * distanceY) <= (radius * radius);
      }
      inline Circle operator+(const Pos<R_FLOAT>& pos) const {Circle retval(*this); retval.center += pos; return retval;}
      Rect<R_FLOAT> circumscribe(){return {center.x-radius, center.y-radius, radius, radius};}
      Pos<R_FLOAT> center;
      R_FLOAT radius;
   };

   struct CircleProperty : public Property<Circle>{
      using Property<ReyEngine::Circle>::operator=;
      CircleProperty(const std::string& instanceName,  ReyEngine::Circle&& defaultvalue=Circle({},0))
      : Property<ReyEngine::Circle>(instanceName, PropertyTypes::Color, std::move(defaultvalue))
      {}
      std::string toString() const override {
         auto fvec = value.center.getElements();
         fvec.push_back(value.radius);
         return string_tools::listJoin(fvec);
      }
      ReyEngine::Circle fromString(const std::string& str) override {
         auto split = string_tools::fromList(str);
         return {Pos<R_FLOAT>(STOF(split.at(0)), STOF(split.at(1))), STOF(split.at(2))};
      }
   };

   struct CircleSector : public Circle {
      static constexpr double FIRST_QUADRANT_ANGLE = 0;
      static constexpr double SECOND_QUADRANT_ANGLE = 90;
      static constexpr double THIRD_QUADRANT_ANGLE = 180;
      static constexpr double FOURTH_QUADRANT_ANGLE = 270;
      static CircleSector firstQuadrant(Pos<int> center, double radius){return CircleSector(center, radius, FIRST_QUADRANT_ANGLE, FIRST_QUADRANT_ANGLE+90);}
      static CircleSector secondQuadrant(Pos<int> center, double radius){return CircleSector(center, radius, SECOND_QUADRANT_ANGLE, SECOND_QUADRANT_ANGLE+90);}
      static CircleSector thirdQuadrant(Pos<int> center, double radius){return CircleSector(center, radius, THIRD_QUADRANT_ANGLE, THIRD_QUADRANT_ANGLE+90);}
      static CircleSector fourthQuadrant(Pos<int> center, double radius){return CircleSector(center, radius, FOURTH_QUADRANT_ANGLE, FOURTH_QUADRANT_ANGLE+90);}
      inline CircleSector(const Pos<int>& center, double radius, double startAngle, double endAngle): Circle(center, radius), startAngle(startAngle), endAngle(endAngle){}
      inline CircleSector(const Circle& c, double startAngle, double endAngle): Circle(c), startAngle(startAngle), endAngle(endAngle){}
      inline CircleSector(const CircleSector& rhs): Circle(rhs), startAngle(rhs.startAngle), endAngle(rhs.endAngle){}
      double startAngle;
      double endAngle;
   };

   //should maybe use eigen transforms. one day.
   struct Transform2D {
      Vec2<R_FLOAT> translation;
      R_FLOAT rotation; // In radians
      Vec2<R_FLOAT> scale = {1.0f, 1.0f};

      // Get final position of a point after transform
      Vec2<float> transform(const Vec2<float>& point) const {
         float cos_a = std::cos(rotation);
         float sin_a = std::sin(rotation);
         return Vec2<float>{
               point.x * cos_a - point.y * sin_a + translation.x,
               point.x * sin_a + point.y * cos_a + translation.y
         };
      }

      // Compose two transforms, returning the equivalent single transform
//      Transform2D operator*(const Transform2D& rhs) const {
//         // First rotate rhs translation by our rotation
//         float cos_a = std::cos(rotation);
//         float sin_a = std::sin(rotation);
//         Vec2<float> rotated_translation{
//               rhs.translation.x * cos_a - rhs.translation.y * sin_a,
//               rhs.translation.x * sin_a + rhs.translation.y * cos_a
//         };
//
//         return Transform2D{
//               rotation + rhs.rotation,
//               translation + rotated_translation
//         };
//      }
//      Transform2D& operator*=(const Transform2D& rhs) {*this = *this * rhs; return *this;}
//      Transform2D& operator=(const Transform2D& rhs) {
//         if (this != &rhs) {
//            rotation = rhs.rotation;
//            translation = rhs.translation;
//         }
//         return *this;
//      }
//      Transform2D inverse() const {
//         // For rotation, we negate the angle
//         float inv_rotation = -rotation;
//
//         // For translation, we need to rotate it by -angle and negate it
//         float cos_a = std::cos(inv_rotation);
//         float sin_a = std::sin(inv_rotation);
//         Vec2<float> inv_translation{
//               -(translation.x * cos_a - translation.y * sin_a),
//               -(translation.x * sin_a + translation.y * cos_a)
//         };
//
//         return Transform2D{inv_rotation, inv_translation, {0}};
//      }

      Matrix getMatrix() const {
         auto m = MatrixTranslate(translation.x, translation.y, 0);
         m = MatrixMultiply(m, MatrixRotate({0, 0, 1}, rotation));
         m = MatrixMultiply(m, MatrixScale(scale.x, scale.y, 1));
         return m;
      };

      //applies the transform to a point
      Pos<R_FLOAT> transformPoint(const Pos<R_FLOAT>& point) const {
         auto res = Vector3Transform({point.x, point.y, 0}, getMatrix());
         return {res.x, res.y};
      }
      //de-applies the transform to a point
      Pos<R_FLOAT> invertPoint(const Pos<R_FLOAT>& point) const {
         auto res = Vector3Transform({point.x, point.y, 0}, MatrixInvert(getMatrix()));
         return {res.x, res.y};
      }
   };

   struct Transform2DProperty : public Property<Transform2D>{
      using Property<Transform2D>::operator=;
      Transform2DProperty(const std::string& instanceName,  Transform2D&& defaultvalue={})
      : Property<Transform2D>(instanceName, PropertyTypes::Color, std::move(defaultvalue))
      {}
      std::string toString() const override {return "{}";}
      Transform2D fromString(const std::string& str) override {return Transform2D();}
   }; 

   struct ColorRGBA {
      ColorRGBA(): r(0), g(0), b(0), a(255){}
      constexpr ColorRGBA(int r, int g, int b, int a): r(r), g(g), b(b), a(a){}
      ColorRGBA(Color color): r(color.r), g(color.g), b(color.b), a(color.a){}
      ColorRGBA(Color& color): r(color.r), g(color.g), b(color.b), a(color.a){}
      ColorRGBA(Color&& color): r(color.r), g(color.g), b(color.b), a(color.a){}
      inline ColorRGBA& operator=(const Color& rhs){r = rhs.r; g=rhs.g; b=rhs.b; a=rhs.a; return *this;}
      inline operator Color() const {return {r, g, b, a};}
//      inline void setR(unsigned char _r){r = _r;}
//      inline void setG(unsigned char _g){g = _g;}
//      inline void setB(unsigned char _b){b = _b;}
//      inline void setA(unsigned char _a){a = _a;}
      inline static ColorRGBA random(int alpha = -1){
         auto retval = ColorRGBA(std::rand() % 255, std::rand() % 256, std::rand() % 256, alpha >= 0 ? alpha % 256 : std::rand() % 256);
         return retval;
      }
//      friend Color operator=(const Color lhs, ColorRGBA rhs){r = rhs.r; g=rhs.g; b=rhs.b; a=rhs.a; return *this;}
//      friend std::ostream& operator<<(std::ostream& os, Vec2<T> v) {os << v.toString(); return os;}
      unsigned char r;
      unsigned char g;
      unsigned char b;
      unsigned char a;
   };

   struct ColorProperty : public Property<ReyEngine::ColorRGBA>{
      using Property<ReyEngine::ColorRGBA>::operator=;
      ColorProperty(const std::string& instanceName,  ReyEngine::ColorRGBA defaultvalue)//pass color by copy
      : Property<ReyEngine::ColorRGBA>(instanceName, PropertyTypes::Color, std::move(defaultvalue))
      {}
      std::string toString() const override {return "{" + std::to_string(value.r) + ", " + std::to_string(value.g) + ", " + std::to_string(value.b) + ", "  + std::to_string(value.a) + "}";}
      ReyEngine::ColorRGBA fromString(const std::string& str) override {
         auto split = string_tools::fromList(str);
         return {std::stoi(split[0]), std::stoi(split[1]), std::stoi(split[2]), std::stoi(split[3])};
      }
   };


#define COLORS ReyEngine::Colors
   namespace Colors{
      static constexpr ColorRGBA gray = {130, 130, 130, 255};
      static constexpr ColorRGBA lightGray = {200, 200, 200, 255};
      static constexpr ColorRGBA red = {230, 41, 55, 255};
      static constexpr ColorRGBA green = { 0, 228, 48, 255};
      static constexpr ColorRGBA blue = { 0, 121, 241, 255};
      static constexpr ColorRGBA black = { 0, 0, 0, 255};
      static constexpr ColorRGBA yellow = {253, 249, 0, 255};
      static constexpr ColorRGBA orange = {255, 165, 0, 255};
      static constexpr ColorRGBA purple = {127, 0, 127, 255};
      static constexpr ColorRGBA white = {255, 255, 255, 255};
      static constexpr ColorRGBA transparent = {0, 0, 0, 0};
      static constexpr ColorRGBA none = {255, 255, 255, 255};
      static inline ColorRGBA randColor(){return {std::rand() % 255, std::rand() % 255, std::rand() % 255, 255};}  //not very random
   }

   enum class FontAlignmentHorizontal{LEFT, CENTER, RIGHT, /*JUSTIFIED*/};
   enum class FontAlignmentVertical{TOP, CENTER, BOTTOM};

   struct FontAlignment{
      FontAlignmentHorizontal horizontal = FontAlignmentHorizontal::LEFT;
      FontAlignmentVertical vertical = FontAlignmentVertical::TOP;
   };

   struct ReyEngineFont{
      ReyEngineFont(const std::string& fontFile="");
      Font font;
      double size = 20;
      double spacing = 1;
      bool isDefault = false;
      FontAlignment fontAlignment;
      ReyEngine::ColorRGBA color = COLORS::black;
      std::string fileName;
      Size<R_FLOAT> measure(const std::string& text) const;
      ReyEngineFont& operator=(const ReyEngineFont& rhs);
//      inline ReyEngineFont& operator=(const ReyEngineFont& rhs){font = rhs.font; size = rhs.size; spacing = rhs.spacing; color = rhs.color; fileName = rhs.fileName; isDefault = rhs.isDefault; return *this;}
//      std::string toString();
//      static ReyEngineFont fromString(const std::string& str);
   };
   ReyEngineFont getDefaultFont(std::optional<R_FLOAT> fontSize = std::nullopt);
   ReyEngineFont getFont(const std::string& fileName);

   struct ReyTexture{
      ReyTexture(){};
      ReyTexture(const FileSystem::File& file);
       ReyTexture(ReyTexture&& other) noexcept
      :  size(other.size)
      , _tex(other._tex)
      , _texLoaded(other._texLoaded)
      {
         other._texLoaded = false;
         _file = other._file;
      }
      void loadTexture(const FileSystem::File& file);
      ~ReyTexture(){
         if (_texLoaded) {
            UnloadTexture(_tex);
         }
         _file.clear();
      }
      const Texture2D& getTexture() const {return _tex;}
      operator bool() const {return _texLoaded;}
      std::string getPath(){return _file;}
      Size<int> size;
   protected:
      FileSystem::File _file;
      Texture2D _tex;
      bool _texLoaded = false;
   };

   Pos<R_FLOAT> getScreenCenter();
   Size<int> getScreenSize();
   ReyEngine::Size<int> getWindowSize();
   void setWindowSize(ReyEngine::Size<int>);
   ReyEngine::Pos<int> getWindowPosition();
   void setWindowPosition(ReyEngine::Pos<int>);
   void maximizeWindow();
   void minimizeWindow();
   void drawText(const std::string& text, const Pos<R_FLOAT>& pos, const ReyEngineFont& font);
   void drawTextCentered(const std::string& text, const Pos<R_FLOAT>& pos, const ReyEngineFont& font);
   void drawTextRelative(const std::string& text, const Pos<R_FLOAT>& relPos, const ReyEngineFont& font);
   void drawRectangle(const Rect<R_FLOAT>&, const ReyEngine::ColorRGBA& color);
   void drawRectangleRounded(const Rect<float>&, float roundness, int segments, const ReyEngine::ColorRGBA& color);
   void drawRectangleLines(const Rect<float>&, float lineThick, const ReyEngine::ColorRGBA& color);
   void drawRectangleRoundedLines(const Rect<float>&, float roundness, int segments, float lineThick, const ReyEngine::ColorRGBA& color);
   void drawRectangleGradientV(const Rect<R_FLOAT>&, const ReyEngine::ColorRGBA& color1, const ReyEngine::ColorRGBA& color2);
   void drawCircle(const Circle&, const ReyEngine::ColorRGBA&  color);
   void drawCircleLines(const Circle&, const ReyEngine::ColorRGBA&  color);
   void drawCircleSector(const CircleSector&, const ReyEngine::ColorRGBA&  color, int segments);
   void drawCircleSectorLines(const CircleSector&, const ReyEngine::ColorRGBA&  color, int segments);
   void drawLine(const Line<R_FLOAT>&, float lineThick, const ReyEngine::ColorRGBA& color);
   void drawArrow(const Line<R_FLOAT>&, float lineThick, const ReyEngine::ColorRGBA& color, float headSize=20); //Head drawn at A
   void drawTexture(const ReyTexture& texture, const Rect<R_FLOAT>& source, const Rect<R_FLOAT>& dest, float rotation, const ReyEngine::ColorRGBA& tint);
   inline float getFrameDelta() {return GetFrameTime();}
   inline Size<R_FLOAT> measureText(const std::string& text, const ReyEngineFont& font){return MeasureTextEx(font.font, text.c_str(), font.size, font.spacing);}

   class RenderTarget{
      public:
         explicit RenderTarget();
         ~RenderTarget();
         void setSize(const Size<int>& newSize);
         inline Size<int> getSize() const {return _size;}
         inline void beginRenderMode(){BeginTextureMode(_tex);}
         inline void endRenderMode(){EndTextureMode();}
         inline void clear(Color color=WHITE) const {ClearBackground(color);}
         inline bool ready() const {return _texLoaded;}
         [[nodiscard]] inline const Texture2D& getRenderTexture() const {return _tex.texture;}
      protected:
         bool _texLoaded = false;
         RenderTexture2D _tex;
         Size<int> _size;
      };

//   struct CameraStack2D{
//      CameraStack2D();
//      Camera2D camera;
//      Pos<R_FLOAT> screenToWorld(const Pos<R_FLOAT>& pos) const {return GetScreenToWorld2D((Vector2)pos, camera);}
//      Pos<R_FLOAT> worldToScreen(const Pos<R_FLOAT>& pos) const {return GetWorldToScreen2D((Vector2)pos, camera);}
//      void push() const;
//      void pop();
//   };
}

namespace InputInterface{
   //Corresponds to ascii table
   enum class KeyCode{
      KEY_NULL            = 0,
      KEY_APOSTROPHE      = 39,       // Key: '
      KEY_COMMA           = 44,       // Key: ,
      KEY_MINUS           = 45,       // Key: -
      KEY_PERIOD          = 46,       // Key: .
      KEY_SLASH           = 47,       // Key: /
      KEY_ZERO            = 48,       // Key: 0
      KEY_ONE             = 49,       // Key: 1
      KEY_TWO             = 50,       // Key: 2
      KEY_THREE           = 51,       // Key: 3
      KEY_FOUR            = 52,       // Key: 4
      KEY_FIVE            = 53,       // Key: 5
      KEY_SIX             = 54,       // Key: 6
      KEY_SEVEN           = 55,       // Key: 7
      KEY_EIGHT           = 56,       // Key: 8
      KEY_NINE            = 57,       // Key: 9
      KEY_SEMICOLON       = 59,       // Key: ;
      KEY_EQUAL           = 61,       // Key: =
      KEY_A               = 65,       // Key: A | a
      KEY_B               = 66,       // Key: B | b
      KEY_C               = 67,       // Key: C | c
      KEY_D               = 68,       // Key: D | d
      KEY_E               = 69,       // Key: E | e
      KEY_F               = 70,       // Key: F | f
      KEY_G               = 71,       // Key: G | g
      KEY_H               = 72,       // Key: H | h
      KEY_I               = 73,       // Key: I | i
      KEY_J               = 74,       // Key: J | j
      KEY_K               = 75,       // Key: K | k
      KEY_L               = 76,       // Key: L | l
      KEY_M               = 77,       // Key: M | m
      KEY_N               = 78,       // Key: N | n
      KEY_O               = 79,       // Key: O | o
      KEY_P               = 80,       // Key: P | p
      KEY_Q               = 81,       // Key: Q | q
      KEY_R               = 82,       // Key: R | r
      KEY_S               = 83,       // Key: S | s
      KEY_T               = 84,       // Key: T | t
      KEY_U               = 85,       // Key: U | u
      KEY_V               = 86,       // Key: V | v
      KEY_W               = 87,       // Key: W | w
      KEY_X               = 88,       // Key: X | x
      KEY_Y               = 89,       // Key: Y | y
      KEY_Z               = 90,       // Key: Z | z
      KEY_LEFT_BRACKET    = 91,       // Key: [
      KEY_BACKSLASH       = 92,       // Key: '\'
      KEY_RIGHT_BRACKET   = 93,       // Key: ]
      KEY_GRAVE           = 96,       // Key: `
            // Function keys
      KEY_SPACE           = 32,       // Key: Space
      KEY_ESCAPE          = 256,      // Key: Esc
      KEY_ENTER           = 257,      // Key: Enter
      KEY_TAB             = 258,      // Key: Tab
      KEY_BACKSPACE       = 259,      // Key: Backspace
      KEY_INSERT          = 260,      // Key: Ins
      KEY_DELETE          = 261,      // Key: Del
      KEY_RIGHT           = 262,      // Key: Cursor right
      KEY_LEFT            = 263,      // Key: Cursor left
      KEY_DOWN            = 264,      // Key: Cursor down
      KEY_UP              = 265,      // Key: Cursor up
      KEY_PAGE_UP         = 266,      // Key: Page up
      KEY_PAGE_DOWN       = 267,      // Key: Page down
      KEY_HOME            = 268,      // Key: Home
      KEY_END             = 269,      // Key: End
      KEY_CAPS_LOCK       = 280,      // Key: Caps lock
      KEY_SCROLL_LOCK     = 281,      // Key: Scroll down
      KEY_NUM_LOCK        = 282,      // Key: Num lock
      KEY_PRINT_SCREEN    = 283,      // Key: Print screen
      KEY_PAUSE           = 284,      // Key: Pause
      KEY_F1              = 290,      // Key: F1
      KEY_F2              = 291,      // Key: F2
      KEY_F3              = 292,      // Key: F3
      KEY_F4              = 293,      // Key: F4
      KEY_F5              = 294,      // Key: F5
      KEY_F6              = 295,      // Key: F6
      KEY_F7              = 296,      // Key: F7
      KEY_F8              = 297,      // Key: F8
      KEY_F9              = 298,      // Key: F9
      KEY_F10             = 299,      // Key: F10
      KEY_F11             = 300,      // Key: F11
      KEY_F12             = 301,      // Key: F12
      KEY_LEFT_SHIFT      = 340,      // Key: Shift left
      KEY_LEFT_CONTROL    = 341,      // Key: Control left
      KEY_LEFT_ALT        = 342,      // Key: Alt left
      KEY_LEFT_SUPER      = 343,      // Key: Super left
      KEY_RIGHT_SHIFT     = 344,      // Key: Shift right
      KEY_RIGHT_CONTROL   = 345,      // Key: Control right
      KEY_RIGHT_ALT       = 346,      // Key: Alt right
      KEY_RIGHT_SUPER     = 347,      // Key: Super right
      KEY_KB_MENU         = 348,      // Key: KB menu
            // Keypad keys
      KEY_KP_0            = 320,      // Key: Keypad 0
      KEY_KP_1            = 321,      // Key: Keypad 1
      KEY_KP_2            = 322,      // Key: Keypad 2
      KEY_KP_3            = 323,      // Key: Keypad 3
      KEY_KP_4            = 324,      // Key: Keypad 4
      KEY_KP_5            = 325,      // Key: Keypad 5
      KEY_KP_6            = 326,      // Key: Keypad 6
      KEY_KP_7            = 327,      // Key: Keypad 7
      KEY_KP_8            = 328,      // Key: Keypad 8
      KEY_KP_9            = 329,      // Key: Keypad 9
      KEY_KP_DECIMAL      = 330,      // Key: Keypad .
      KEY_KP_DIVIDE       = 331,      // Key: Keypad /
      KEY_KP_MULTIPLY     = 332,      // Key: Keypad *
      KEY_KP_SUBTRACT     = 333,      // Key: Keypad -
      KEY_KP_ADD          = 334,      // Key: Keypad +
      KEY_KP_ENTER        = 335,      // Key: Keypad Enter
      KEY_KP_EQUAL        = 336,      // Key: Keypad =
            // Android key buttons
      KEY_BACK            = 4,        // Key: Android back button
      KEY_MENU            = 82,       // Key: Android menu button
      KEY_VOLUME_UP       = 24,       // Key: Android volume up button
      KEY_VOLUME_DOWN     = 25        // Key: Android volume down button
   };

//   using KeyCode = KeyCode;
   enum class MouseButton{
      NONE = -1,
      LEFT = MOUSE_BUTTON_LEFT,
      RIGHT = MOUSE_BUTTON_RIGHT,
      BACK = MOUSE_BUTTON_BACK,
      EXTRA = MOUSE_BUTTON_EXTRA,
      FORWARD = MOUSE_BUTTON_FORWARD,
      MIDDLE = MOUSE_BUTTON_MIDDLE,
      SIDE = MOUSE_BUTTON_SIDE,
   };

   //array of all mouse buttons
   static constexpr std::array<MouseButton, 7> MouseButtons = {
         MouseButton::LEFT,
         MouseButton::RIGHT,
         MouseButton::MIDDLE,
         MouseButton::BACK,
         MouseButton::FORWARD,
         MouseButton::SIDE,
         MouseButton::EXTRA,
   };

   enum class MouseCursor{
      DEFAULT = MOUSE_CURSOR_DEFAULT,
      ARROW = MOUSE_CURSOR_ARROW,
      IBEAM = MOUSE_CURSOR_IBEAM,
      CROSSHAIR = MOUSE_CURSOR_CROSSHAIR,
      POINTING_HAND = MOUSE_CURSOR_POINTING_HAND,
      RESIZE_EW = MOUSE_CURSOR_RESIZE_EW,
      RESIZE_NS = MOUSE_CURSOR_RESIZE_NS,
      RESIZE_NWSE = MOUSE_CURSOR_RESIZE_NWSE,
      RESIZE_NESW = MOUSE_CURSOR_RESIZE_NESW,
      RESIZE_ALL = MOUSE_CURSOR_RESIZE_ALL,
      NOT_ALLOWED = MOUSE_CURSOR_NOT_ALLOWED
   };

   inline constexpr std::optional<char> toChar(KeyCode keyCode){ return (static_cast<int>(keyCode) <= 127) ? std::optional<char>{static_cast<char>(static_cast<int>(keyCode))} : std::nullopt;}

//   // Define the friend function
//   ReyEngine::Logger::Stream& operator<<(const InputInterface::KeyCode& keyCode, ReyEngine::Logger& logger) {
//      auto optKey = toChar(keyCode);
//      logger << (optKey ? std::string(optKey.value(), 1) : std::string("Unsupported Char")) << std::endl;
//   }

   inline float getMouseWheelMove(){return GetMouseWheelMove();} //returns largest of x or y
   inline ReyEngine::Vec2<R_FLOAT> getMouseWheelMoveV(){return GetMouseWheelMoveV();} //returns both x and y

   inline bool isKeyPressed(KeyCode key){return IsKeyPressed((int)key);}
   inline bool isKeyDown(KeyCode key){return IsKeyDown((int)key);}
   inline bool isKeyReleased(KeyCode key){return IsKeyReleased((int)key);}
   inline bool isKeyUp(KeyCode key){return IsKeyUp((int)key);}
   inline void setExitKey(KeyCode key){return SetExitKey((int)key);}
   inline KeyCode getKeyPressed(){return (KeyCode)GetKeyPressed();}
   inline char getCharPressed(){return GetCharPressed();}
   inline bool isMouseButtonPressed(MouseButton btn){return IsMouseButtonPressed(static_cast<int>(btn));}
   inline bool isMouseButtonDown(MouseButton btn){return IsMouseButtonDown(static_cast<int>(btn));}
   inline bool isMouseButtonUp(MouseButton btn){return IsMouseButtonUp(static_cast<int>(btn));}
   inline bool isMouseButtonReleased(MouseButton btn){return IsMouseButtonReleased(static_cast<int>(btn));}
   inline ReyEngine::Vec2<R_FLOAT> getMousePos(){return GetMousePosition();}
   inline ReyEngine::Vec2<R_FLOAT> getMouseDelta(){return GetMouseDelta();}
   inline ReyEngine::Vec2<R_FLOAT> getMouseWheel(){return GetMouseWheelMoveV();}

   inline void setCursor(MouseCursor crsr){ SetMouseCursor((int)crsr);}
   inline void hideCursor(){HideCursor();}
}

namespace DisplayInterface {
    inline void toggleFullscreen(){ToggleFullscreen();}
}