#pragma once
#include "raylib.h"
#include <stdexcept>
#include "StringTools.h"
#include <string>
#include <array>
#include <iostream>
#include "FileSystem.h"
#ifdef linux
#include <limits.h>
#endif


#define NOT_IMPLEMENTED throw std::runtime_error("Not implemented!")

namespace ReyEngine {
   static constexpr long long MaxInt = INT_MAX;
   static constexpr long long MinInt = INT_MIN;
   template <typename T> struct Size;
   template <typename T> struct Pos;
   template <typename T>
   struct Vec {
      explicit Vec(size_t size): size(size){}
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
      inline Vec2(): Vec<T>(2), x(0), y(0){}
      inline Vec2(const T& x, const T& y) : Vec<T>(2), x(x), y(y){}
      inline Vec2(const Vector2& v)     : Vec<T>(2), x((T)v.x), y((T)v.y){}
      inline Vec2(const Vec2<int>& v)   : Vec<T>(2), x((T)v.x), y((T)v.y){}
      inline Vec2(const Vec2<float>& v) : Vec<T>(2), x((T)v.x), y((T)v.y){}
      inline Vec2(const Vec2<double>& v): Vec<T>(2), x((T)v.x), y((T)v.y){}
      inline explicit operator bool() const {return x || y;}
      inline Vec2 operator+(const Vec2& rhs) const {Vec2<T> val = *this; val.x += rhs.x; val.y += rhs.y; return val;}
      inline Vec2 operator-(const Vec2& rhs) const {Vec2<T> val = *this; val.x -= rhs.x; val.y -= rhs.y; return val;}
      inline Vec2& operator+=(const Vec2& rhs){x += rhs.x; y += rhs.y; return *this;}
      inline Vec2& operator-=(const Vec2& rhs){x -= rhs.x; y -= rhs.y; return *this;}
      inline Vec2& operator*=(const Vec2& rhs){x *= rhs.x; y *= rhs.y; return *this;}
      inline Vec2& operator/=(const Vec2& rhs){x /= rhs.x; y /= rhs.y; return *this;}
      inline Vec2& operator=(const Vec2& rhs){x = rhs.x; y=rhs.y; return *this;}
      inline bool operator==(const Vec2& rhs){return x==rhs.x && y==rhs.y;}
      inline Vec2& operator-(){x = -x; y =-y; return *this;}
      inline void operator=(Size<T>&) = delete;
      inline void operator=(Pos<T>&) = delete;
      inline operator Vector2() const {return {(float)x,(float)y};}
      inline Vec2 midpoint(){return {x/2, y / 2};}
      inline void min(Vec2<T> other){if (this->x > other.x) this->x = other.x; if (this->y > other.y) this->y = other.y;}
      inline void max(Vec2<T> other){if (this->x < other.x) this->x = other.x; if (this->y < other.y) this->y = other.y;}
      inline double pct(double input){return (input-x)/(y - x);} //given an input value, what percentage of the range is it from 0 to 1?
      inline double lerp(double lerpVal){return lerpVal * (y - x) + x;} //given a value from 0 to 1, what is the value of the range that corresponds to it?
      inline Vec2 lerp(Vec2 otherPoint, double xprm){return {xprm, y + (((xprm - x) * (otherPoint.y - y)) / (otherPoint.x - x))};}
      inline T clamp(T value){if (value < x) return x; if (value > y) return y; return value;}
      inline Vec2 clamp(Vec2 clampA, Vec2 clampB){
         Vec2 retval = {x, y};
         if (x < clampA.x) retval.x = clampA.x;
         if (x > clampB.x) retval.x = clampB.x;
         if (y < clampA.x) retval.y = clampA.y;
         if (y > clampB.y) retval.y = clampB.y;
         return retval;
      }
      inline static std::vector<T> fromString(const std::string& s){return Vec<T>::fromString(2, s);};
      std::ostream& operator<<(std::ostream& os) const {os << Vec<T>::toString(); return os;}
      friend std::ostream& operator<<(std::ostream& os, Vec2<T> v) {os << v.toString(); return os;}
      T x;
      T y;
   protected:
      [[nodiscard]] inline std::vector<T> getElements() const override {return {x,y};}
   };

   template <typename T>
   struct Vec3 : protected Vec<T> {
      inline Vec3(): Vec<T>(3), x(0), y(0), z(0){}
       inline Vec3(const T& _x, const T& y, const T& _z) : Vec<T>(3), x(_x), y(y),z(_z) {}
      inline explicit Vec3(const Vector3& v)     : Vec<T>(3), x((T)v.x), y((T)v.y), z((T)v.z){}
      inline explicit Vec3(const Vec3<int>& v)   : Vec<T>(3), x((T)v.x), y((T)v.y), z((T)v.z){}
      inline explicit Vec3(const Vec3<float>& v) : Vec<T>(3), x((T)v.x), y((T)v.y), z((T)v.z){}
      inline explicit Vec3(const Vec3<double>& v): Vec<T>(3), x((T)v.x), y((T)v.y), z((T)v.z){}
      inline Vec3& operator=(const Vec3& rhs){x = rhs.x; y=rhs.y; z=rhs.z; return *this;}
      inline Vec3& operator-(){x = -x; y =-y; z = -z; return *this;}
      inline static std::vector<T> fromString(const std::string& s){return Vec<T>::fromString(3, s);};
      [[nodiscard]] inline std::vector<T> getElements() const override {return {x,y,z};}
      friend std::ostream& operator<<(std::ostream& os, Vec3<T> v) {os << v.toString(); return os;}
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
        inline Vec4(const Vec4<int>& v)        : Vec<T>(4), w((T)v.w), x((T)v.x), y((T)v.y), z((T)v.z){}
        inline Vec4(const Vec4<float>& v)      : Vec<T>(4), w((T)v.w), x((T)v.x), y((T)v.y), z((T)v.z){}
        inline Vec4(const Vec4<double>& v)     : Vec<T>(4), w((T)v.w), x((T)v.x), y((T)v.y), z((T)v.z){}
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
      //Vec3, but x represents min, z represents max, and y represents value; Can lerp and set pct.
      //When creating from Vec3, vec3 shall be considered x=min, y=max, z=default value.
      //Does not enforce limits (ie value is free to be more or less than min or max.
      inline Range(): Vec3<T>::Vec3(){}
      inline Range(const T& min, const T& max, const T& defaultValue) : Vec3<T>::Vec3(min, max, defaultValue){}
      inline explicit Range(const Vector3& v)     : Vec3<T>::Vec3(v){}
      inline explicit Range(const Vec3<int>& v)   : Vec3<T>::Vec3(v){}
      inline explicit Range(const Vec3<float>& v) : Vec3<T>::Vec3(v){}
      inline explicit Range(const Vec3<double>& v): Vec3<T>::Vec3(v){}
      inline double getpct(){return getRange().pct(getValue());};
      inline void setLerp(double pct){setValue(getRange().lerp(pct));}
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
      inline Line(): a(0,0), b(0,0){}
      inline Line(Pos<T> a, Pos<T> b): a(a), b(b){}
      inline Line(const T x1, const T y1, const T x2, const T y2): Line({x1, y1}, {x2, y2}){}
      inline Line midpoint(){return {(a.x+b.x)/2, (a.y + b.y) / 2};}
      inline Pos<T> lerp(double xprm){return a.lerp(b, xprm);}
      inline double distance(){NOT_IMPLEMENTED;}
      inline std::string toString() const {return "{" + a.toString() + ", " + b.toString() + "}";}
      Pos<T> a;
      Pos<T> b;
   };

   template <typename T>
   struct Pos : public Vec2<T>{
      inline Pos(): Vec2<T>(){}
      inline Pos(const T& x, const T& y) : Vec2<T>(x, y){}
      inline Pos(const Vector2& v)     : Vec2<T>(v){}
      inline Pos(const Vec2<int>& v)   : Vec2<T>(v){}
      inline Pos(const Vec2<double>& v): Vec2<T>(v){}
      inline Pos(const Vec2<float>& v) : Vec2<T>(v){}

      inline Pos operator+(const Pos& rhs) const {auto val = *this; val.x += rhs.x; val.y += rhs.y; return val;}
      inline Pos operator-(const Pos& rhs) const {auto val = *this; val.x -= rhs.x; val.y -= rhs.y; return val;}
      inline Pos& operator+=(const Pos& rhs){this->x += rhs.x; this->y += rhs.y; return *this;}
      inline Pos& operator-=(const Pos& rhs){this->x -= rhs.x; this->y -= rhs.y; return *this;}
      inline bool operator!=(const Pos& rhs){return this->x != rhs.x || this->y != rhs.y;}
      inline operator std::string() const {return Vec2<T>::toString();}
      inline void operator=(const Size<T>&) = delete;
      inline Pos& operator=(const Pos<T>& other){Pos::x = other.x; Pos::y = other.y; return *this;}
      inline Pos& operator=(const Vec2<T>& other){Pos::x = other.x; Pos::y = other.y; return *this;}
      inline std::string toString() const {return Vec2<T>::toString();}
   };

   template <typename T>
   struct Size : public Vec2<T>{
      inline Size(): Vec2<T>(){}
      inline Size(const T& x, const T& y) : Vec2<T>(x, y){}
      inline Size(const Vector2& v)     : Vec2<T>(v.x,v.y){}
      inline Size(const Vec2<int>& v)   : Vec2<T>(v.x,v.y){}
      inline Size(const Size<int>& v)   : Vec2<T>(v){}
      inline Size(const Size<double>& v): Vec2<T>(v){}
      inline Size(const Size<float>& v) : Vec2<T>(v){}
      inline void operator=(Pos<T>&) = delete;
      inline bool operator==(const Size<T>& rhs){return Size::x==rhs.x && Size::y==rhs.y;}
      inline bool operator!=(const Size<T>& rhs){return Size::x!=rhs.x || Size::y!=rhs.y;}
      inline Size operator+(const Size& rhs) const {auto val = *this; val.x += rhs.x; val.y += rhs.y; return val;}
      inline Size operator-(const Size& rhs) const {auto val = *this; val.x -= rhs.x; val.y -= rhs.y; return val;}
      inline Size& operator+=(const Size& rhs){this->x += rhs.x; this->y += rhs.y; return *this;}
      inline Size& operator-=(const Size& rhs){this->x -= rhs.x; this->y -= rhs.y; return *this;}
      inline operator std::string() const {return Vec2<T>::toString();}
   };

   template <typename T>
   struct Rect {
      inline Rect(): x(0), y(0), width(0), height(0){}
      inline Rect(const T x, const T y, const T width, const T height) : x(x), y(y), width(width), height(height){}
      inline explicit Rect(const Rectangle& r): x((T)r.x), y((T)r.y), width((T)r.width), height((T)r.height){}
      inline Rect(const Rect<int>& r): x((T)r.x), y((T)r.y), width((T)r.width), height((T)r.height){}
      inline Rect(const Rect<float>& r): x((T)r.x), y((T)r.y), width((T)r.width), height((T)r.height){}
      inline Rect(const Rect<double>& r): x((T)r.x), y((T)r.y), width((T)r.width), height((T)r.height){}
      inline explicit Rect(const Vec2<T>&) = delete;
      inline explicit Rect(const Pos<T>& v): x((T)v.x), y((T)v.y){}
      inline explicit Rect(const Size<T>& v): width((T)v.x), height((T)v.y){}
      inline operator bool(){return x || y || width || height;}
      inline Rect(const Pos<T>& pos, const Size<T>& size): x((T)pos.x), y((T)pos.y), width((T)size.x), height((T)size.y){}
      inline bool operator!=(const Rect<T>& rhs) const {return rhs.x != x || rhs.y != y || rhs.width != width || rhs.height != height;}
      inline Rect operator+(const Pos<T>& rhs) const {Rect<T> val = *this; val.x += rhs.x; val.y += rhs.y; return val;}
      inline Rect operator-(const Pos<T>& rhs) const {Rect<T> val = *this; val.x -= rhs.x; val.y -= rhs.y; return val;}
      inline Rect& operator+=(const Pos<T>& rhs){x += rhs.x; y += rhs.y; return *this;}
      inline Rect& operator-=(const Pos<T>& rhs){x -= rhs.x; y -= rhs.y; return *this;}
      inline Rect& operator*=(const Pos<T>& rhs){x *= rhs.x; y *= rhs.y; return *this;}
      inline Rect& operator/=(const Pos<T>& rhs){x /= rhs.x; y /= rhs.y; return *this;}
      inline Rect operator+(const Size<T>& rhs) const {Rect<T> val = *this; val.width += rhs.x; val.height += rhs.y; return val;}
      inline Rect operator-(const Size<T>& rhs) const {Rect<T> val = *this; val.width -= rhs.x; val.height -= rhs.y; return val;}
      inline Rect& operator+=(const Size<T>& rhs){width += rhs.width; height += rhs.height; return *this;}
      inline Rect& operator-=(const Size<T>& rhs){width -= rhs.width; height -= rhs.height; return *this;}
      inline Rect& operator*=(const Size<T>& rhs){width *= rhs.width; height *= rhs.height; return *this;}
      inline Rect& operator/=(const Size<T>& rhs){width /= rhs.width; height /= rhs.height; return *this;}
      inline Rect operator+(const Rect<T>& rhs) const {Rect<T> val = *this; val.x += rhs.x; val.y += rhs.y; val.width += rhs.width; val.height += rhs.height; return val;}
      inline Rect operator-(const Rect<T>& rhs) const {Rect<T> val = *this; val.x -= rhs.x; val.y -= rhs.y; val.width -= rhs.width; val.height -= rhs.height; return val;}
      inline Rect& operator+=(const Rect<T>& rhs){x += rhs.x; y += rhs.y; width += rhs.width; height += rhs.height; return *this;}
      inline Rect& operator-=(const Rect<T>& rhs){x -= rhs.x; y -= rhs.y; width -= rhs.width; height -= rhs.height; return *this;}
      inline Rect& operator*=(const Rect<T>& rhs){x *= rhs.x; y *= rhs.y; width *= rhs.width; height *= rhs.height; return *this;}
      inline Rect& operator/=(const Rect<T>& rhs){x /= rhs.x; y /= rhs.y; width /= rhs.width; height /= rhs.height; return *this;}
      inline operator Rectangle() {return {x,y,width,height};}
      inline bool isInside(const Vec2<T>& point) const {
         return (point.x > x && point.x < x + width) &&
               (point.y > y && point.y < y + height);
      }
      [[nodiscard]] inline Vec2<T> center() const {return {(x+width)/2, (y+height)/2};}
      inline void setCenter(const Vec2<T>& center) {x = center.x-width/2; y= center.y - height / 2;}
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
      [[nodiscard]] inline const Pos<T> pos() const {return {x, y};}
      [[nodiscard]] inline const Size<T> size() const {return {width, height};}
      [[nodiscard]] inline const Rect<T> toSizeRect() const {return {0,0,width, height};}
      inline void setSize(const ReyEngine::Size<T>& size){width = size.x; height = size.y;}
      inline void setPos(const ReyEngine::Pos<T>& pos){x = pos.x; y = pos.y;}

      T x;
      T y;
      T width;
      T height;
   };

   struct Circle{
      inline Circle(Pos<int> center, double radius): center(center), radius(radius){}
      inline Circle(const Circle& rhs): center(rhs.center), radius(rhs.radius){}
      Pos<int> center;
      double radius;
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

   struct ColorRGBA {
      ColorRGBA(): r(0), g(0), b(0), a(255){}
      constexpr ColorRGBA(int r, int g, int b, int a): r(r), g(g), b(b), a(a){}
      explicit ColorRGBA(Color color): r(color.r), g(color.g), b(color.b), a(color.a){}
      inline ColorRGBA& operator=(const Color& rhs){r = rhs.r; g=rhs.g; b=rhs.b; a=rhs.a; return *this;}
      inline operator Color() const {return {r, g, b, a};}
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

   #define COLORS ReyEngine::Colors
   namespace Colors{
      static constexpr ColorRGBA gray = {130, 130, 130, 255};
      static constexpr ColorRGBA lightGray = {200, 200, 200, 255};
      static constexpr ColorRGBA red = {230, 41, 55, 255};
      static constexpr ColorRGBA green = { 0, 228, 48, 255};
      static constexpr ColorRGBA blue = { 0, 121, 241, 255};
      static constexpr ColorRGBA black = { 0, 0, 0, 255};
      static constexpr ColorRGBA yellow = {253, 249, 0, 255};
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
      float size = 20;
      float spacing = 1;
      bool isDefault = false;
      FontAlignment fontAlignment;
      ReyEngine::ColorRGBA color = COLORS::black;
      std::string fileName;
      Size<int> measure(const std::string& text) const;
      ReyEngineFont& operator=(const ReyEngineFont& rhs);
//      inline ReyEngineFont& operator=(const ReyEngineFont& rhs){font = rhs.font; size = rhs.size; spacing = rhs.spacing; color = rhs.color; fileName = rhs.fileName; isDefault = rhs.isDefault; return *this;}
//      std::string toString();
//      static ReyEngineFont fromString(const std::string& str);
   };
   ReyEngineFont getDefaultFont();
   ReyEngineFont getFont(const std::string& fileName);

   struct ReyTexture{
      ReyTexture(FileSystem::File file);
      ~ReyTexture(){
         if (_texLoaded) {
            UnloadTexture(_tex);
         }
      }
      const Texture2D& getTexture() const {return _tex;}
      operator bool() const {return _texLoaded;}
      Size<int> size;
   protected:
      Texture2D _tex;
      bool _texLoaded;
   };

   Pos<double> getScreenCenter();
   Size<int> getScreenSize();
   ReyEngine::Size<int> getWindowSize();
   void setWindowSize(ReyEngine::Size<int>);
   ReyEngine::Pos<int> getWindowPosition();
   void setWindowPosition(ReyEngine::Pos<int>);
   void maximizeWindow();
   void minimizeWindow();
   void drawText(const std::string& text, const ReyEngine::Pos<int>& pos, const ReyEngineFont& font);
   void drawTextCentered(const std::string& text, const ReyEngine::Pos<int>& pos, const ReyEngineFont& font);
   void drawTextRelative(const std::string& text, const ReyEngine::Pos<int>& relPos, const ReyEngineFont& font);
   void drawRectangle(const Rect<int>&, const ReyEngine::ColorRGBA& color);
   void drawRectangleRounded(const Rect<float>&, float roundness, int segments, const ReyEngine::ColorRGBA& color);
   void drawRectangleLines(const Rect<float>&, float lineThick, const ReyEngine::ColorRGBA& color);
   void drawRectangleRoundedLines(const Rect<float>&, float roundness, int segments, float lineThick, const ReyEngine::ColorRGBA& color);
   void drawRectangleGradientV(const Rect<int>&, ReyEngine::ColorRGBA& color1, const ReyEngine::ColorRGBA& color2);
   void drawCircleSector(const CircleSector&, const ReyEngine::ColorRGBA&  color, int segments);
   void drawCircleSectorLines(const CircleSector&, const ReyEngine::ColorRGBA&  color, int segments);
   void drawLine(const Line<int>&, const ReyEngine::ColorRGBA& color);
   void drawTexture(const ReyTexture& texture, const Rect<int>& source, const Rect<int>& dest, float rotation, const ReyEngine::ColorRGBA& tint);
   inline float getFrameDelta() {return GetFrameTime();}
   inline Size<int> measureText(const std::string& text, ReyEngineFont font){return MeasureTextEx(font.font, text.c_str(), font.size, font.spacing);}

   class RenderTarget{
      public:
         explicit RenderTarget();
         ~RenderTarget(){
            if (_texLoaded) {
               UnloadRenderTexture(_tex);
            }
         }
         void setSize(const Size<int>& newSize);
         void beginRenderMode(){BeginTextureMode(_tex);}
         void endRenderMode(){EndTextureMode();}
         void clear(Color color=WHITE){ClearBackground(color);}
         void render(Vec2<float> pos) const{
            // NOTE: Render texture must be y-flipped due to default OpenGL coordinates (left-bottom)
            if (_texLoaded) {
               DrawTextureRec(_tex.texture, {pos.x, pos.y, (float) _size.x, (float) -_size.y}, {0, 0}, Colors::none);
            }
         }
      protected:
         bool _texLoaded = false;
         RenderTexture2D _tex;
         Size<int> _size;
      };

   //Everything drawn duiring scissor mode will be invisible if outside the area
   class ScissorTarget {
   public:
      void start(Rect<double> scissorArea){BeginScissorMode((int)scissorArea.x, (int)scissorArea.y, (int)scissorArea.width, (int)scissorArea.height);}
      void stop(){EndScissorMode();}
   };
}

namespace InputInterface{
   using KeyCode = int;
   enum class MouseButton{
      MOUSE_BUTTON_NONE = -1,
      MOUSE_BUTTON_LEFT = MOUSE_BUTTON_LEFT,
      MOUSE_BUTTON_RIGHT = MOUSE_BUTTON_RIGHT,
      MOUSE_BUTTON_BACK = MOUSE_BUTTON_BACK,
      MOUSE_BUTTON_EXTRA = MOUSE_BUTTON_EXTRA,
      MOUSE_BUTTON_FORWARD = MOUSE_BUTTON_FORWARD,
      MOUSE_BUTTON_MIDDLE = MOUSE_BUTTON_MIDDLE,
      MOUSE_BUTTON_SIDE = MOUSE_BUTTON_SIDE,
   };

   //array of all mouse buttons
   static constexpr MouseButton MouseButtons[] = {
         MouseButton::MOUSE_BUTTON_LEFT,
         MouseButton::MOUSE_BUTTON_RIGHT,
         MouseButton::MOUSE_BUTTON_MIDDLE,
         MouseButton::MOUSE_BUTTON_BACK,
         MouseButton::MOUSE_BUTTON_FORWARD,
         MouseButton::MOUSE_BUTTON_SIDE,
         MouseButton::MOUSE_BUTTON_EXTRA,
   };

   inline float getMouseWheelMove(){return GetMouseWheelMove();} //returns largest of x or y
   inline ReyEngine::Vec2<float> getMouseWheelMoveV(){return GetMouseWheelMoveV();} //returns both x and y

   inline bool isKeyPressed(KeyCode key){return IsKeyPressed(key);}
   inline bool isKeyDown(KeyCode key){return IsKeyDown(key);}
   inline bool isKeyReleased(KeyCode key){return IsKeyReleased(key);}
   inline bool isKeyUp(KeyCode key){return IsKeyUp(key);}
   inline void setExitKey(KeyCode key){return SetExitKey(key);}
   inline KeyCode getKeyPressed(){return GetKeyPressed();}
   inline KeyCode getCharPressed(){return GetCharPressed();}
   inline bool isMouseButtonPressed(MouseButton btn){return IsMouseButtonPressed(static_cast<int>(btn));}
   inline bool isMouseButtonDown(MouseButton btn){return IsMouseButtonDown(static_cast<int>(btn));}
   inline bool isMouseButtonUp(MouseButton btn){return IsMouseButtonUp(static_cast<int>(btn));}
   inline bool isMouseButtonReleased(MouseButton btn){return IsMouseButtonReleased(static_cast<int>(btn));}
   inline ReyEngine::Vec2<int> getMousePos(){return GetMousePosition();}
   inline ReyEngine::Vec2<int> getMouseDelta(){return GetMouseDelta();}
}