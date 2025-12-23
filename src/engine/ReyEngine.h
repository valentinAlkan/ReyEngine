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
#include "StrongUnits.h"
#ifdef linux
#include <limits.h>
#endif

#define R_FLOAT float //float or double?
#define STOF std::stof
#define NOT_IMPLEMENTED throw std::runtime_error("Not implemented!")
// Helper for dependent static_assert in templates
template<typename> inline constexpr bool always_false_v = false;

namespace ReyEngine {
   enum WindowFlags{RESIZE, IS_EDITOR};
   static constexpr long long MaxInt = INT_MAX;
   static constexpr long long MinInt = INT_MIN;
   static constexpr R_FLOAT MaxFloat = FLT_MAX;
   static constexpr R_FLOAT MinFloat = FLT_MIN;

   namespace Math {
      template <typename T> T min(T a, T b){return a <= b ? a : b;}
      template <typename T> T max(T a, T b){return a >= b ? a : b;}
   }

   class FNVHash {
      static constexpr std::size_t FNV_PRIME = 0x100000001b3;
      static constexpr std::size_t FNV_OFFSET = 0xcbf29ce484222325;
   public:
      static constexpr std::size_t hash(std::string_view str) {std::size_t hash = FNV_OFFSET; for (char c: str) {hash ^= static_cast<std::size_t>(c); hash *= FNV_PRIME;} return hash;}
   };

   class DJB2Hash {
   public:
      static constexpr std::size_t hash(std::string_view str) {std::size_t hash = 5381;for (char c: str) {hash = ((hash << 5) + hash) + static_cast<std::size_t>(c);}return hash;}
   };

   inline std::ostream& operator<<(std::ostream& os, const Matrix& m) {
      os << "[" << std::fixed << std::setprecision(3) << m.m0 << "  " << m.m1 << "  " << m.m2 << "  " << m.m3 << "]\n";
      os << "[" << std::fixed << std::setprecision(3) << m.m4 << "  " << m.m5 << "  " << m.m6 << "  " << m.m7 << "]\n";
      os << "[" << std::fixed << std::setprecision(3) << m.m8 << "  " << m.m9 << "  " << m.m10 << "  " << m.m11 << "]\n";
      os << "[" << std::fixed << std::setprecision(3) << m.m12 << "  " << m.m13 << "  " << m.m14 << "  " << m.m15 << "]\n";
      return os;
   }
   inline void printMatrix(const Matrix& m) {
      // Print in row-major format for readability
      printf("Matrix (column-major):\n");
      std::cout << m << std::endl;
      fflush(stdout);
   }
   inline std::string matrixToString(const Matrix& m) {
      std::stringstream ss;
      ss << m;
      return ss.str();
   }

   // Canvas Coordinates - relative to current canvas
   template <typename T>
   struct WindowSpace{
   public:
      WindowSpace() = default;
      WindowSpace(const T& other): underlying(other){}
      WindowSpace& operator=(const T& other){ underlying = other;}
      T& get(){return underlying;}
      const T& get() const {return underlying;}
      friend std::ostream& operator<<(std::ostream& os, const WindowSpace<T>& other) {os << other.get(); return os;}

   private:
      T underlying;
   };

   // Canvas Coordinates - relative to current canvas
   template <typename T>
   struct CanvasSpace{
   public:
      CanvasSpace() = default;
      CanvasSpace(const T& other): underlying(other){}
      CanvasSpace& operator=(const T& other){ underlying = other;}
      T& get(){return underlying;}
      const T& get() const {return underlying;}
      friend std::ostream& operator<<(std::ostream& os, const CanvasSpace<T>& other) {os << other.get(); return os;}
   private:
      T underlying;
   };

   /// This struct aids in remembering values. Using it like type T should always use the 'first' value,
   /// but you can swap between two values, or march the values down the line to 'remember' the x most recent values
   /// values[0] is most recent value, values.back() is least recent value
   template <typename T, unsigned int COUNT=2>
   struct MemoryValue{
      MemoryValue() requires std::is_trivially_constructible_v<T> {
         values.fill(T{});
      };
      MemoryValue(const T& value) : values{} {
         values.fill(T{});
         values[0] = value;
      }
      template<typename ...Args>
      MemoryValue(Args &&... args)
      requires (sizeof...(args) > 1) && (sizeof...(args) == COUNT)
      : values{static_cast<T>(std::forward<Args>(args))...}
      {}
      constexpr operator T() const { return values[0]; }
      constexpr operator T&() { return values[0]; }
      constexpr operator const T&() const { return values[0]; }
      constexpr T& operator=(const T& other){march(); values[0] = other; return values[0];}
      constexpr void swap() requires(COUNT == 2) {std::swap(values[0], values[1]);}
      constexpr void march() requires(COUNT >= 2) {
         if constexpr (COUNT > 2) {
            T back = values[COUNT - 1];
            for (std::size_t i = COUNT - 1; i > 0; --i) {
               values[i] = values[i - 1];
            }
            values[0] = back;
            return;
         }
         //otherwise do a swap
         swap();
      }
      constexpr void setIfNot(const T& other){if (mostRecent() != other) *this = other;}
      constexpr T operator[](unsigned int i){return values[i];}
      constexpr T at(unsigned int i){return values.at(i);}
      constexpr T mostRecent() const {return values[0];}
      std::array<T, COUNT> values;
   };

   template <typename T> struct Size;
   template <typename T> struct Pos;
   template <typename T> struct Rect;
   template <typename T, uint32_t SIZE>
   struct Vec {
   protected:
      template<typename R>
      inline static std::optional<R> fromString(const std::string &s) {
         std::string sanitized;
         for (const auto &c: s) {
            if (::isdigit(c) || c == '-' || c == ',' || c == '.') {
               sanitized += c;
            }
         }

         auto split = string_tools::split(sanitized, ",");
         if (split.size() != SIZE) {
            return {};
         }

         try {
            return makeFromStrings<R>(split, std::make_index_sequence<SIZE>{});
         } catch (const std::exception&) {
            return {}; // Parse error
         }
      }

      template<typename ...Args>
      [[nodiscard]] static inline std::string _toString(Args &&... args) {
         std::string retval = "{";
         auto addToString = [&retval](const auto &arg) {
            retval += std::to_string(arg);
            retval += ", ";
         };

         (addToString(args), ...);

         //remove trailing space and comma if needed
         if (sizeof...(args) > 0) {
            retval.pop_back();
            retval.pop_back();
         }
         retval += "}";
         return retval;
      }

   private:
      template<typename R, size_t... Is>
      static R makeFromStrings(const std::vector<std::string>& strings, std::index_sequence<Is...>) {
         return R{parseValue(strings[Is])...};
      }

      static T parseValue(const std::string& str) {
         if constexpr (std::is_same_v<T, float>) {
            return std::stof(str);
         } else if constexpr (std::is_same_v<T, double>) {
            return std::stod(str);
         } else if constexpr (std::is_integral_v<T>) {
            return static_cast<T>(std::stoll(str));
         } else {
            static_assert(std::is_arithmetic_v<T>, "Unsupported type for parsing");
         }
      }
   };

   struct UnitVector2;
   template <typename T>
   struct Vec2 : protected Vec<T, 2> {
      constexpr inline Vec2(): Vec<T, 2>(), x(0), y(0){}
      constexpr inline Vec2(const Vec2& other) : Vec<T, 2>(), x(other.x), y(other.y) {}
      constexpr inline Vec2(Vec2&& other) noexcept : Vec<T, 2>(), x(std::move(other.x)), y(std::move(other.y)) {}
      constexpr inline Vec2(const T& x, const T& y) : Vec<T, 2>(), x(x), y(y){}
      constexpr inline Vec2(const Vector2& v) : Vec<T, 2>(), x((T)v.x), y((T)v.y){}
      constexpr inline Vec2& operator=(Vec2&& rhs) noexcept { x = std::move(rhs.x); y = std::move(rhs.y); return *this; }
      template <typename R>
      constexpr inline Vec2(const Vec2<R>& v): Vec<T, 2>(), x((T)v.x), y((T)v.y){}
      constexpr inline explicit operator bool() const {return x || y;}
      constexpr inline Vec2 operator+(const Vec2& rhs) const {Vec2<T> val = *this; val.x += rhs.x; val.y += rhs.y; return val;}
      constexpr inline Vec2 operator-(const Vec2& rhs) const {Vec2<T> val = *this; val.x -= rhs.x; val.y -= rhs.y; return val;}
      constexpr inline Vec2& operator+=(const Vec2& rhs){x += rhs.x; y += rhs.y; return *this;}
      constexpr inline Vec2& operator-=(const Vec2& rhs){x -= rhs.x; y -= rhs.y; return *this;}
      constexpr inline Vec2 operator*(double rhs) const {Vec2 retval(*this); retval.x *= rhs; retval.y *= rhs; return retval;}
//      inline Vec2& operator*=(const Vec2& rhs){x *= rhs.x; y *= rhs.y; return *this;}
      constexpr inline Vec2& operator*=(double rhs){x *= rhs; y *= rhs; return *this;}
      constexpr inline Vec2<R_FLOAT> operator/(double rhs) const {Vec2<R_FLOAT> retval(*this); retval.x /= rhs; retval.y /= rhs; return retval;}
      constexpr inline Vec2& operator/=(const Vec2& rhs){x /= rhs.x; y /= rhs.y; return *this;}
      constexpr inline Vec2& operator=(const Vec2& rhs){x = rhs.x; y=rhs.y; return *this;}
      constexpr inline bool operator==(const Vec2& rhs) const {return x==rhs.x && y==rhs.y;}
      constexpr inline bool operator!=(const Vec2& rhs) const {return x!=rhs.x || y!=rhs.y;}
      constexpr inline Vec2 operator-() const {return {-x, -y};}
      constexpr inline void operator=(Size<T>&) = delete;

      inline void operator=(Pos<T>&) = delete;
      inline constexpr operator Vector2() const {return {(float)x,(float)y};}
      [[nodiscard]] constexpr inline T magnitude() const {return std::sqrt(x * x + y * y);}
      static constexpr inline T magnitude(T x, T y) {return std::sqrt(x * x + y * y);}
      [[nodiscard]] UnitVector2 direction(const Vec2<T>& dest) const; //get the unit vector that points at dest from this point's perspective
//      constexpr inline Vec2 midpoint() const {return {x/2, y / 2};} does this make sense?
      [[nodiscard]] constexpr inline Vec2 min(const Vec2& other) const {Vec2 r; r.x = Math::min(Vec2::x, other.x); r.y = Math::min(Vec2::y, other.y); return r;}
      [[nodiscard]] constexpr inline Vec2 max(const Vec2& other) const {Vec2 r; r.x = Math::max(Vec2::x, other.x); r.y = Math::max(Vec2::y, other.y); return r;}
      [[nodiscard]] constexpr inline Fraction pct(R_FLOAT input) const {return (input - x) / (y - x);} //given an input value, what percentage of the range is it from 0 to 1?
      [[nodiscard]] constexpr inline R_FLOAT lerp(Fraction f) const {return f.get() * (y - x) + x;} //given a value from 0 to 1, what is the value of the range that corresponds to it?
      [[nodiscard]] constexpr inline Vec2 lerp(const Vec2& otherPoint, Fraction f) const {return *this + (otherPoint - *this) * f.get();}
      [[nodiscard]] constexpr inline Vec2 extend(R_FLOAT distance) const {Vec2<T> normalized = normalize();return normalized * distance;}
      [[nodiscard]] constexpr inline T clamp(T value) const {if (value < x) return x; if (value > y) return y; return value;}
      [[nodiscard]] constexpr inline Vec2 clamp(Vec2 clampA, Vec2 clampB) const {
         Vec2 retval = {x, y};
         if (x < clampA.x) retval.x = clampA.x;
         if (x > clampB.x) retval.x = clampB.x;
         if (y < clampA.x) retval.y = clampA.y;
         if (y > clampB.y) retval.y = clampB.y;
         return retval;
      }
      constexpr inline Vec2<T> rotate(const Radians& r) const {
         T cosA = std::cos(r.get());
         T sinA = std::sin(r.get());
         return Vec2<T>(x * cosA - y * sinA,x * sinA + y * cosA);
      }
      [[nodiscard]] constexpr inline R_FLOAT length() const {return length(x, y);}
      static constexpr inline double length(T x, T y) {return std::sqrt(x * x + y * y);}
      static constexpr inline Vec2<T> normalize(T x, T y) {
         T magnitude = std::sqrt(x * x + y * y);
         if (magnitude == 0) {return Vec2<T>(0, 0);}
         return Vec2<T>(x / magnitude, y / magnitude);
      }
      inline Vec2<T> normalize() const {return normalize(x, y);}
      inline static std::optional<Vec2<T>> fromString(const std::string& s){return Vec<T, 2>::template fromString<Vec2<T>>(s);}
      friend std::ostream& operator<<(std::ostream& os, Vec2<T> v) {os << v.toString(); return os;}
      friend Vector2& operator+=(Vector2& in, Vec2<T> add) {in.x += add.x; in.y += add.y; return in;}
      inline Vec2& transform(const Matrix& m) {
         auto retval = Vector3Transform({x, y, 0}, m);
         x = retval.x;
         y = retval.y;
         return *this;
      }
      static inline Vec2 transform(const Vec2& v, const Matrix& m) {
         auto retval = Vector3Transform({v.x, v.y, 0}, m);
         return Vec2(retval.x, retval.y);
      }
      [[nodiscard]] constexpr inline Vec2 transform(const Matrix& m) const {
         auto retval = Vector3Transform({x, y, 0}, m);
         return Vec2(retval.x, retval.y);
      }
      Pos<T> toPos() const {return {x, y};}
      Size<T> toSize() const {return {x, y};}
      inline std::string toString() const {return Vec<T, 2>::_toString(x, y);}
      T x;
      T y;
   };

   // A unit vector. If the magnitude is not 1, then the vector is invalid.
   struct CircleSector;
   struct UnitVector2 {
      constexpr UnitVector2()=default;
      constexpr UnitVector2(const Vec2<R_FLOAT>& v): UnitVector2(v.x, v.y){}
      constexpr UnitVector2(R_FLOAT x, R_FLOAT y){
         auto normalized = Vec2<R_FLOAT>::normalize(x, y);
         _x = normalized.x;
         _y = normalized.y;
      }
      constexpr Vec2<R_FLOAT> operator*(R_FLOAT distance) const {return {_x*distance, _y*distance};}
      constexpr UnitVector2& operator=(const Vec2<R_FLOAT>& v){*this = UnitVector2(v); return *this;}
      constexpr operator Vec2<R_FLOAT>() const {return {_x,_y};}
      [[nodiscard]] constexpr Vec2<R_FLOAT> toVec2() const {return (Vec2<R_FLOAT>)(*this);}
      [[nodiscard]] bool valid() const  {return FloatEquals(toVec2().magnitude(), 1.0);}
      [[nodiscard]] constexpr R_FLOAT x() const {return _x;}
      [[nodiscard]] constexpr R_FLOAT y() const {return _y;}
      [[nodiscard]] Vec2<int> ortho4() const {
         if (std::abs(_x) > std::abs(_y)) {
            return {_x > 0 ? 1 : -1, 0};
         } else {
            return {0, _y > 0 ? 1 : -1};
         }
      }
      [[nodiscard]] Vec2<int> ortho8() const {
         constexpr float DIAGONAL_THRESHOLD = 0.9239f; //22.5 degrees
         float absX = std::abs(_x);
         float absY = std::abs(_y);
         if (absX / absY > DIAGONAL_THRESHOLD && absY / absX > DIAGONAL_THRESHOLD) {
            //diagonals
            return {_x > 0 ? 1 : -1,_y > 0 ? 1 : -1};
         }
         //fallback to cardinal directions
         return ortho4();
      }
      [[nodiscard]] auto rotate(const Radians& r) const {
         return toVec2().rotate(r);
      }
      [[nodiscard]] CircleSector toCircleSector(Degrees totalAngle, double radius, const Pos<R_FLOAT>& pos) const;
      friend std::ostream& operator<<(std::ostream& os, const UnitVector2& v) {os << v.toVec2().toString(); return os;}
   private:
      R_FLOAT _x=0;
      R_FLOAT _y=0;
   };

   template <typename T>
   struct Vec3 : protected Vec<T, 3> {
      constexpr inline Vec3(): Vec<T, 3>(), x(0), y(0), z(0){}
      constexpr inline operator Vector3() const {return {(float)x,(float)y,(float)z};}
      constexpr inline Vec3(const T& _x, const T& y, const T& _z) : Vec<T, 3>(), x(_x), y(y),z(_z) {}
      constexpr inline explicit Vec3(const Vector3& v)     : Vec<T, 3>(), x((T)v.x), y((T)v.y), z((T)v.z){}
      constexpr inline Vec3(const Vec3& v): Vec<T, 3>(), x((T)v.x), y((T)v.y), z((T)v.z){}
      constexpr inline Vec3& operator=(const Vec3& rhs){x = rhs.x; y=rhs.y; z=rhs.z; return *this;}
      constexpr inline Vec3& operator-(){x = -x; y =-y; z = -z; return *this;}
      constexpr inline Vec3 operator-(const Vec3& rhs) const {Vec3 retval; retval.x=x-rhs.x; retval.y=y-rhs.y; retval.z=z-rhs.z; return retval;}
      constexpr inline Vec3 operator+(const Vec3& rhs) const {Vec3 retval; retval.x=x+rhs.x; retval.y=y+rhs.y; retval.z=z+rhs.z; return retval;}
      constexpr inline Vec3& operator-=(const Vec3& rhs){x-=rhs.x; y-=rhs.y; z-=rhs.z; return *this;}
      constexpr inline Vec3& operator+=(const Vec3& rhs){x+=rhs.x; y+=rhs.y; z+=rhs.z; return *this;}
      inline std::optional<Vec3<T>> normalize() const {return normalize(*this);}
      constexpr inline static Vec3<T> normalize(const Vec3& other) {
         auto len = other.magnitude();
         if (len == T(0)) return {};
         return Vec3(other.x / len, other.y / len, other.z / len);
      }
      inline T magnitude() const {return std::sqrt(x * x + y * y + z * z);}
      inline T dot(const Vec3& rhs) const {return x * rhs.x + y * rhs.y + z * rhs.z;}
      inline static T dot(const Vec3& a, const Vec3& b){return a.dot(b);}
      inline Vec3<T> cross(const Vec3& rhs) const {return {y * rhs.z - z * rhs.y,z * rhs.x - x * rhs.z,x * rhs.y - y * rhs.x};}
      inline static Vec3<T> cross(const Vec3& a, const Vec3& b){return a.cross(b);}
      constexpr inline std::string toString(){return Vec<T, 3>::_toString(x, y, z);}
      inline static std::optional<Vec3<T>> fromString(const std::string& s){return Vec<T,3>::fromString(s);};
      friend std::ostream& operator<<(std::ostream& os, const Vec3& v) {os << v.toString(); return os;}
      T x;
      T y;
      T z;
   };

   template <typename T>
   struct Vec4 : protected Vec<T, 4> {
      constexpr inline Vec4(): Vec<T, 4>(), w(0), x(0), y(0), z(0){}
      constexpr inline Vec4(const std::vector<T>& stdvec): Vec<T, 4>(){
         if (stdvec.size() != 4) throw std::runtime_error("Invalid element count for Vec4! Expected 4, got " + stdvec.size());
         w = stdvec[0];
         x = stdvec[1];
         y = stdvec[2];
         z = stdvec[3];
      }
      constexpr inline Vec4(const T& _w, const T& _x, const T& y, const T& _z) : Vec<T, 4>(), w(_w), x(_x), y(y),z(_z) {}
      constexpr inline explicit Vec4(const Vector4& v) : Vec<T, 4>(), w((T)v.w), x((T)v.x), y((T)v.y), z((T)v.z){}
      template <typename R>
      constexpr inline Vec4(const Vec4<R>& v) : Vec<T, 4>(), w((T)v.w), x((T)v.x), y((T)v.y), z((T)v.z){}
      constexpr inline Vec4& operator=(const Vec4& rhs){w = rhs.w, x = rhs.x; y=rhs.y; z=rhs.z; return *this;}
      constexpr inline Vec4& operator-(){w = -w; x = -x; y =-y; z = -z; return *this;}
      constexpr inline static std::optional<Vec4<T>> fromString(const std::string& s){return Vec<T, 4>::fromString(s);};
      inline std::string toString(){return Vec<T, 4>::_toString(w, x, y, z);}
      constexpr friend std::ostream& operator<<(std::ostream& os, Vec4 v) {os << v.toString(); return os;}
      T w;
      T x;
      T y;
      T z;
   };


   struct UnitVector3 {
      constexpr UnitVector3()=default;
      constexpr UnitVector3(const Vec3<R_FLOAT>& v): UnitVector3(v.x, v.y, v.z){}
      constexpr UnitVector3(R_FLOAT x, R_FLOAT y, R_FLOAT z){
         auto normalized = Vec3<R_FLOAT>::normalize({x, y, z});
         _x = normalized.x;
         _y = normalized.y;
         _z = normalized.z;
      }
      constexpr Vec3<R_FLOAT> operator*(R_FLOAT distance) const {return {_x*distance, _y*distance, _z*distance};}
      constexpr UnitVector3& operator=(const Vec3<R_FLOAT>& v){*this = UnitVector3(v); return *this;}
      constexpr operator Vec3<R_FLOAT>() const {return {_x, _y, _z};}
      constexpr Vec3<R_FLOAT> toVec3() const {return (Vec3<R_FLOAT>)(*this);}
      bool valid() const {return FloatEquals(toVec3().magnitude(), 1.0);}
      constexpr R_FLOAT x() const {return _x;}
      constexpr R_FLOAT y() const {return _y;}
      constexpr R_FLOAT z() const {return _z;}

      // Project to primary axes (similar to ortho4 in 2D)
      Vec3<int> ortho6() const {
         float absX = std::abs(_x);
         float absY = std::abs(_y);
         float absZ = std::abs(_z);

         if (absX >= absY && absX >= absZ) {
            return {_x > 0 ? 1 : -1, 0, 0};
         } else if (absY >= absX && absY >= absZ) {
            return {0, _y > 0 ? 1 : -1, 0};
         } else {
            return {0, 0, _z > 0 ? 1 : -1};
         }
      }

      // Project to primary axes or diagonals (similar to ortho8 in 2D, but in 3D it's ortho26)
      // This is a simplified version that only handles primary axes and main diagonals (8 directions)
      Vec3<int> ortho26() const {
         constexpr float DIAGONAL_THRESHOLD = 0.7071f; // 45 degrees
         float absX = std::abs(_x);
         float absY = std::abs(_y);
         float absZ = std::abs(_z);

         bool strongX = absX > DIAGONAL_THRESHOLD;
         bool strongY = absY > DIAGONAL_THRESHOLD;
         bool strongZ = absZ > DIAGONAL_THRESHOLD;

         return {
               strongX ? (_x > 0 ? 1 : -1) : 0,
               strongY ? (_y > 0 ? 1 : -1) : 0,
               strongZ ? (_z > 0 ? 1 : -1) : 0
         };
      }

      // Rotate around an arbitrary axis
      // This is a placeholder - implementation would depend on your rotation system
      void rotate(const Vec3<R_FLOAT>& axis, const Radians& angle) {
         // Would need a proper 3D rotation implementation
         // This depends on how your Vec3 implements rotation
      }

      // Cross product with another unit vector (returns a unit vector)
      UnitVector3 cross(const UnitVector3& other) const {
         auto result = toVec3().cross(other.toVec3());
         return UnitVector3(result);
      }

      // Dot product with another unit vector
      R_FLOAT dot(const UnitVector3& other) const {
         return _x * other._x + _y * other._y + _z * other._z;
      }
      friend std::ostream& operator<<(std::ostream& os, const UnitVector3& v) {
         os << v.toVec3().toString();
         return os;
      }
   private:
      R_FLOAT _x=0;
      R_FLOAT _y=0;
      R_FLOAT _z=0;
   };

   template <typename T>
   struct Range : private Vec3<T> {
   public:
      //Vec3, but x represents min, y represents max, and z represents value; Can lerp and set pct.
      //When creating from Vec3, vec3 shall be considered x=min, y=max, z=default value.
      //Does not enforce limits (ie value is free to be more or less than min or max.
      inline Range(): Vec3<T>::Vec3(){}
      inline Range(const T& min, const T& max, const T& defaultValue) : Vec3<T>::Vec3(min, max, defaultValue){}
      inline explicit Range(const Vector3& v)     : Vec3<T>::Vec3(v){}
      template <typename R>
      inline explicit Range(const Vec3<R>& v)   : Vec3<T>::Vec3(v){}
      inline Fraction getpct(){return getRange().pct(getValue());};
      inline void setLerp(Fraction pct){setValue(getRange().lerp(pct));}
      inline T getMin() const {return Vec3<T>::x;}
      inline T getMax() const {return Vec3<T>::y;}
      inline T getValue() const {return Vec3<T>::z;}
      inline T setMin(T value){return Vec3<T>::x = value;}
      inline T setMax(T value){return Vec3<T>::y = value;}
      inline T setValue(T value){return Vec3<T>::z = value;}
      inline Vec2<T> getRange(){return Vec2<T>(Vec3<T>::x, Vec3<T>::y);}
      inline void setRange(Vec2<T> newRange){ setMin(newRange.x); setMax(newRange.y);}
      inline void setRange(T min, T max){ setMin(min); setMax(max);}
      /// Wraps the value with modulo-like functionality
      inline T mod(){
         auto range = Vec3<T>::y - Vec3<T>::x;
         if (range <= 0) return Vec3<T>::x;
         auto value = Vec3<T>::z;
         if (value >= Vec3<T>::x && value < Vec3<T>::y) return value;
         auto offset = value - Vec3<T>::x;
         auto wrapped = offset % range;
         if (wrapped < 0) wrapped += range;
         return Vec3<T>::x + wrapped;
      }
      inline static void fromString(const std::string& s){return Vec3<T>::fromString();};
      friend std::ostream& operator<<(std::ostream& os, Range<T> r) {os << r.toString(); return os;}
   };

   template <typename T>
   struct Line {
      constexpr Line(): a(0,0), b(0,0){}
      template <typename _t>
      constexpr Line(const Line<_t>& other): Line(other.a, other.b){}
      constexpr Line(const Pos<T>& a, const Pos<T>& b): a(a), b(b){}
      constexpr Line(const T x1, const T y1, const T x2, const T y2): Line({x1, y1}, {x2, y2}){}
      [[nodiscard]] constexpr Line copy(){return *this;}
      [[nodiscard]] constexpr Pos<T> midpoint() const {return {a.x/2+b.x/2, a.y/2+b.y/2};}
      [[nodiscard]] constexpr Pos<T> lerp(Fraction x) const {return Pos<T>(a.lerp(b, x.get()));}
      [[nodiscard]] constexpr double distance() const {return a.distanceTo(b);}
      inline std::string toString() const {return "{" + a.toString() + ", " + b.toString() + "}";}
      constexpr Line& operator+=(const Pos<T>& pos){a += pos; b += pos; return *this;}
      [[nodiscard]] constexpr Line operator+(const Pos<T>& pos) const {Line<T> l(*this); l.a += pos; l.b += pos; return l;}
      constexpr Line& operator-=(const Pos<T>& pos){a -= pos; b -= pos; return *this;}
      [[nodiscard]] constexpr Line operator-(const Pos<T>& pos) const {Line<T> l(*this); l.a -= pos; l.b -= pos; return l;}
      constexpr Line& pushX(R_FLOAT amt){a.x+= amt; b.x += amt; return *this;}
      constexpr Line& pushY(R_FLOAT amt){a.y+= amt; b.y += amt; return *this;}
      [[nodiscard]] constexpr inline Line reverse() const {return {b,a};}
      //Find the angle from horizontal between points and a b
      [[nodiscard]] constexpr inline Radians angle() const {
         auto dx = static_cast<R_FLOAT>(b.x - a.x);
         auto dy = static_cast<R_FLOAT>(b.y - a.y);
         return atan2(dy, dx);
      }
      [[nodiscard]]
      //rotate the line around A by r radians
      constexpr inline Line& rotate(Pos<T> basis, Radians r){a.rotatePoint(basis, r); b.rotatePoint(basis, r); return *this;}
      constexpr inline Line& scale(Percent pct) {
         // Calculate the direction vector from a to b
         auto direction = b - a;
         // Scale the direction vector by the percentage
         auto extensionVector = direction * Fraction(pct).get();
         auto newB = a + Pos(extensionVector.x, extensionVector.y);
         b = newB;
         return *this;
      }
      constexpr inline Line& extend(float amt) {
         // Get the direction vector
         auto dir = b - a;
         // Normalize it
         auto length = distance();
         if (length == 0) return *this; // Prevent division by zero

         // Scale the direction vector by the amount to extend
         auto extensionVector = dir * (amt / length);
         // Create a new endpoint by adding the extension vector to b
         auto newB = b + Pos<T>(extensionVector.x, extensionVector.y);
         b = newB;
         return *this;
      }

      constexpr inline Line& shorten(float amt) {return extend(-amt);}
      //project a line a fixed amount from the start point
      constexpr inline Line& project(double amount) {b = a.project(b, amount); return *this;}
      constexpr inline T slope(){
         if (a.x == b.x) throw std::runtime_error("Undefined slope (vertical line)");
         return (b.y - a.y) / (b.x - a.x);
      }
      constexpr inline std::pair<Line, Line> normals(){
         auto dx = b.x - a.x;
         auto dy = b.y - a.y;
         return {{-dy, dx}, {dy, -dx}};
      }
      constexpr inline std::optional<Pos<T>> intersectionOf(const Line<T>& other) const {
         // Get coordinates of the points
         const T x1 = a.x;
         const T y1 = a.y;
         const T x2 = b.x;
         const T y2 = b.y;
         const T x3 = other.a.x;
         const T y3 = other.a.y;
         const T x4 = other.b.x;
         const T y4 = other.b.y;

         // Calculate denominator
         const T denominator = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);

         // Check if lines are parallel (denominator = 0)
         if (denominator == 0) return {};

         // Calculate intersection point
         const T numeratorX = ((x1 * y2 - y1 * x2) * (x3 - x4) - (x1 - x2) * (x3 * y4 - y3 * x4));
         const T numeratorY = ((x1 * y2 - y1 * x2) * (y3 - y4) - (y1 - y2) * (x3 * y4 - y3 * x4));

         return Pos<T>(numeratorX / denominator, numeratorY / denominator);
      }

      // Default split (two equal halves)
      template<bool AsTuple = false>
      [[nodiscard]] auto split() const {
         std::vector<Line<T>> resultVec = splitImpl({});
         if constexpr (AsTuple) {
            return make_tuple_from_vector(resultVec, std::make_index_sequence<2>{});
         } else {
            return resultVec;
         }
      }

      // Split with a single percentage (results in two parts: the percentage, and the remainder)
      template<bool AsTuple = false>
      [[nodiscard]] auto split(const Percent& percent) const {
         std::vector<Percent> percentages{percent};
         std::vector<Line<T>> resultVec = splitImpl(percentages);
         if constexpr (AsTuple) {
            return make_tuple_from_vector(resultVec, std::make_index_sequence<2>{});
         } else {
            return resultVec;
         }
      }

      // Split with a vector of percentages (results in N+1 parts)
      [[nodiscard]] std::vector<Line<T>> split(const std::vector<Percent>& percentages) const {
         return splitImpl(percentages);
      }

      template<bool AsTuple = false, typename... Args, typename = std::enable_if_t<(sizeof...(Args) > 1)>>
      [[nodiscard]] auto split(const Args& ... args) const {
         std::vector<Percent> percentages;
         percentages.reserve(sizeof...(args));
         (percentages.push_back(Percent(static_cast<double>(args))), ...); // Convert args to Percent and add
         std::vector<Line<T>> resultVec = splitImpl(percentages);

         if constexpr (AsTuple) {
            // The number of parts is (number of arguments) + 1 (for the remainder)
            return make_tuple_from_vector(resultVec, std::make_index_sequence<sizeof...(Args) + 1>{});
         } else {
            return resultVec;
         }
      }

      friend std::ostream& operator<<(std::ostream& os, Line r) {os << r.toString(); return os;}
      Pos<T> a;
      Pos<T> b;

   private:
      [[nodiscard]] std::vector<Line<T>> splitImpl(const std::vector<Percent>& percentages) const {
         std::vector<Line<T>> result;

         // If no percentages provided, split into two equal parts
         if (percentages.empty()) {
            result.reserve(2);
            Pos<T> m = midpoint();
            result.push_back({a, m});
            result.push_back({m, b});
            return result;
         }

         // Normal case with percentages provided
         result.reserve(percentages.size() + 1);

         // Calculate the total percentage from the input vector
         double totalPercentage = 0.0;
         for (const auto& percent: percentages) {
            totalPercentage += percent.get();
         }

         // Calculate the remaining percentage for the last line segment
         double remainingPercentage = 100.0 - totalPercentage;

         // Ensure the total doesn't exceed 100% (with a small epsilon for float comparison)
         if (remainingPercentage < -1e-6) {
            return splitImpl({}); // Fallback to default split
         }

         Pos<T> currentStart = a;
         R_FLOAT totalLength = distance();

         // Create line segments for each percentage in the vector
         for (const auto& percent: percentages) {
            R_FLOAT segmentLength = totalLength * (percent.get() / 100.0);

            Pos<T> segmentEnd = currentStart;
            segmentEnd.project(b, segmentLength);

            result.push_back({currentStart, segmentEnd});
            currentStart = segmentEnd;
         }

         // Create the final (N+1)th segment with the remaining percentage, if applicable
         if (remainingPercentage > 1e-6) {
            result.push_back({currentStart, b});
         } else if (remainingPercentage < -1e-6) {
            result.back().b = b;
         }


         return result;
      }

      // Helper function to create a tuple from a vector of Lines
      template<std::size_t... I>
      auto make_tuple_from_vector(const std::vector<Line<T>>& vec, std::index_sequence<I...>) const {
         return std::make_tuple(vec[I]...);
      }

   };

   template <typename T=R_FLOAT>
   struct Pos : public Vec2<T>{
      constexpr inline Pos(): Vec2<T>(){}
      constexpr inline Pos(const T& x, const T& y) : Vec2<T>(x, y){}
      constexpr inline Pos(const Vector2& v) : Vec2<T>(v){}
      template <typename R>
      constexpr explicit inline Pos(const Vec2<R>& v) : Vec2<T>(v){}
      constexpr inline void operator=(Size<T>&) = delete;
      template <typename R>
      constexpr inline Pos& operator=(const Pos<R>& other){Vec2<T>::x = other.x; Vec2<T>::y=other.y; return *this;}
      constexpr inline Pos operator+(const Pos& rhs) const {auto val = *this; val.x += rhs.x; val.y += rhs.y; return val;}
      constexpr inline Pos operator-(const Pos& rhs) const {auto val = *this; val.x -= rhs.x; val.y -= rhs.y; return val;}
      template <typename R>
      constexpr inline Pos operator-(const Vec2<R>& rhs) const {return *this - Pos<T>(rhs);}
      constexpr inline Pos operator-() const {return {-Vec2<T>::x, -Vec2<T>::y};}
      constexpr inline Pos& operator+=(const Pos& rhs){this->x += rhs.x; this->y += rhs.y; return *this;}
      constexpr inline Pos& operator-=(const Pos& rhs){this->x -= rhs.x; this->y -= rhs.y; return *this;}
      constexpr inline bool operator!=(const Pos& rhs){return this->x != rhs.x || this->y != rhs.y;}
      template <typename R>
      constexpr inline Pos operator*(R amt) const {return {static_cast<T>(Vec2<T>::x*amt), static_cast<T>(Vec2<T>::y*amt)};}
      template <typename R>
      constexpr inline Pos operator/(R amt) const {return {Vec2<T>::x/amt, Vec2<T>::y/amt};}
      constexpr inline Pos transform(const Matrix& m){return Pos<T>(Vec2<T>::transform(*this, m));}
      constexpr inline Pos xOnly(){return {Pos::x, 0};}
      constexpr inline Pos yOnly(){return {0, Pos::y};}
      constexpr inline Pos& pushX(R_FLOAT amt) {this->x += amt; return *this;}
      constexpr inline Pos& pushY(R_FLOAT amt) {this->y += amt; return *this;}
      inline static std::optional<Pos<T>> fromString(const std::string& s){
         if (auto optVec2 = Vec2<T>::fromString(s)) return Pos<T>(optVec2.value().x, optVec2.value().y);
         return {};
      }
      inline operator std::string() const {return Vec2<T>::toString();}
      constexpr inline void operator=(const Size<T>&) = delete;
      [[nodiscard]] Rect<T> toRect() const;
      [[nodiscard]] Rect<T> toRect(const Size<T>&) const;
      [[nodiscard]] Rect<T> toCenterRect(const Size<T>&) const;
      [[nodiscard]] constexpr inline Pos clamp(Pos clampA, Pos clampB) const { return Pos(Vec2<T>::clamp(clampA, clampB));}
//      inline Pos& operator=(const Vec2<T>& other){Pos::x = other.x; Pos::y = other.y; return *this;}
//      Rotate around a basis point
      constexpr inline Pos& rotatePoint(const Pos<T>& basis, Radians r) {
         double radians = r.get();
         // Translate point to origin
         double xTranslated = Pos::x - basis.x;
         double yTranslated = Pos::y - basis.y;
         // Apply rotation and translate back
         Pos<int> p_rotated;
         Pos::x = static_cast<T>(xTranslated * cos(radians) - yTranslated * sin(radians) + basis.x);
         Pos::y = static_cast<T>(xTranslated * sin(radians) + yTranslated * cos(radians) + basis.y);
         return *this;
      }
      // Function to project a point distance d from point a along the line ab
      constexpr inline Pos& project(const Pos& b, double d) {
         // Calculate the direction vector from a to b
         Pos direction = b - *this;
         // Normalize the direction vector
         Pos unitDirection = Pos(direction.normalize());
         // Scale the normalized vector by distance d
         Pos scaledDirection = Pos(unitDirection * d);
         // Calculate the new point by adding the scaled direction to point a
         *this += Pos(scaledDirection.x, scaledDirection.y);
         return *this;
      }
      constexpr inline double distanceTo(const Pos& other) const {
         auto diff = *this - other;
         return std::sqrt(diff.x * diff.x + diff.y * diff.y);
      }
      constexpr inline Line<T> to(const Pos& other) const {return Line<T>(*this, other);}
      inline std::string toString() const {return Vec2<T>::toString();}
      bool isInside(const Rect<T>& rect);
   };

//   template <typename T>
//   struct PosProperty : public Property<Pos<T>>{
//      using Property<Pos<T>>::operator=;
//      PosProperty(const std::string& instanceName,  Pos<T>&& defaultvalue={})
//      : Property<Pos<T>>(instanceName, PropertyTypes::Pos, std::move(defaultvalue))
//      {}
//      std::string toString() const override {return "";}
//      Pos<T> fromString(const std::string& str) override {return {};}
//   };

   template <typename T=R_FLOAT>
   struct Size : public Vec2<T>{
      constexpr inline Size(): Vec2<T>(){}
      constexpr inline Size(const T& x, const T& y) : Vec2<T>(x, y){}
      constexpr inline explicit Size(const T& edge): Size(edge, edge){}
      constexpr inline Size(const Vector2& v)     : Vec2<T>(v.x,v.y){}
      template <typename R>
      constexpr inline Size(const Vec2<R>& v)   : Vec2<T>(v.x,v.y){}
//      constexpr inline Size(const Size<T>& v) : Vec2<T>(v){}
      constexpr inline void operator=(Pos<T>&) = delete;
      constexpr inline bool operator==(const Size<T>& rhs) const {return Size::x==rhs.x && Size::y==rhs.y;}
      constexpr inline bool operator!=(const Size<T>& rhs) const {return Size::x!=rhs.x || Size::y!=rhs.y;}
      constexpr inline Size operator+(const Size& rhs) const {auto val = *this; val.x += rhs.x; val.y += rhs.y; return val;}
      constexpr inline Size operator-(const Size& rhs) const {auto val = *this; val.x -= rhs.x; val.y -= rhs.y; return val;}
      constexpr inline Size& operator+=(const Size& rhs){this->x += rhs.x; this->y += rhs.y; return *this;}
      constexpr inline Size& operator-=(const Size& rhs){this->x -= rhs.x; this->y -= rhs.y; return *this;}
      [[nodiscard]] constexpr inline Pos<T> center() const {return {this->x/2.0f,this->y/2.0f};}
      [[nodiscard]] constexpr inline Rect<T> toRect() const {return {{0,0}, {*this}};}
      [[nodiscard]] constexpr inline Rect<T> toRect(const Pos<T>& p) const {return {p, {*this}};}
      inline explicit operator std::string() const {return Vec2<T>::toString();}
      constexpr inline static Size Max(){return {std::numeric_limits<T>::max(),std::numeric_limits<T>::max()};}
   };

//   template <typename T>
//   struct SizeProperty : public Property<Size<T>>{
//      using Property<Size<T>>::operator=;
//      SizeProperty(const std::string& instanceName,  Size<T>&& defaultvalue={})
//      : Property<Size<T>>(instanceName, PropertyTypes::Size, std::move(defaultvalue))
//      {}
//      std::string toString() const override {return "";}
//      Size<T> fromString(const std::string& str) override {return {};}
//   };

   struct Circle;
   struct CircleSector;
   struct ReyEngineFont;
   template <typename T>
   struct Rect {
      using SubRectCoords = Vec2<int>;
      enum class Corner {
         TOP_LEFT = 1, TOP_RIGHT = 2, BOTTOM_RIGHT = 4, BOTTOM_LEFT = 8
      };

      constexpr inline Rect() : x(0), y(0), width(0), height(0) {}

      constexpr inline Rect(const T x, const T y, const T width, const T height) : x(x), y(y), width(width),
                                                                                   height(height) {}

      constexpr inline explicit Rect(const Rectangle& r) : x((T) r.x), y((T) r.y), width((T) r.width),
                                                           height((T) r.height) {}

      template<typename R>
      constexpr inline Rect(const Rect<R>& r): x((T) r.x), y((T) r.y), width((T) r.width), height((T) r.height) {}

      inline explicit Rect(const Vec2<T>&) = delete;

      [[nodiscard]] constexpr inline Rect copy() const { return *this; }

      constexpr inline explicit Rect(const Pos<T>& v) : x((T) v.x), y((T) v.y), width(0), height(0) {}

      constexpr inline explicit Rect(const Size<T>& v) : x(0), y(0), width((T) v.x), height((T) v.y) {}

      constexpr inline operator bool() { return x || y || width || height; }

      constexpr inline Rect(const Pos<T>& pos, const Size<T>& size) : x((T) pos.x), y((T) pos.y), width((T) size.x),
                                                                      height((T) size.y) {}

      constexpr inline operator Rectangle() const { return {x, y, width, height}; }

      constexpr inline bool operator==(const Rect<T>& rhs) const {
         return rhs.x == x && rhs.y == y && rhs.width == width && rhs.height == height;
      }

      constexpr inline bool operator!=(const Rect<T>& rhs) const { return !(*this == rhs); }

      constexpr inline Rect operator+(const Pos<T>& rhs) const {
         Rect<T> val = *this;
         val.x += rhs.x;
         val.y += rhs.y;
         return val;
      }

      constexpr inline Rect operator-(const Pos<T>& rhs) const {
         Rect<T> val = *this;
         val.x -= rhs.x;
         val.y -= rhs.y;
         return val;
      }

      constexpr inline Rect& operator+=(const Pos<T>& rhs) {
         x += rhs.x;
         y += rhs.y;
         return *this;
      }

      constexpr inline Rect& operator-=(const Pos<T>& rhs) {
         x -= rhs.x;
         y -= rhs.y;
         return *this;
      }

      constexpr inline Rect& operator*=(const Pos<T>& rhs) {
         x *= rhs.x;
         y *= rhs.y;
         return *this;
      }

      constexpr inline Rect& operator/=(const Pos<T>& rhs) {
         x /= rhs.x;
         y /= rhs.y;
         return *this;
      }

      constexpr inline Rect operator+(const Size<T>& rhs) const {
         Rect<T> val = *this;
         val.width += rhs.x;
         val.height += rhs.y;
         return val;
      }

      constexpr inline Rect operator-(const Size<T>& rhs) const {
         Rect<T> val = *this;
         val.width -= rhs.x;
         val.height -= rhs.y;
         return val;
      }

      constexpr inline Rect& operator+=(const Size<T>& rhs) {
         width += rhs.width;
         height += rhs.height;
         return *this;
      }

      constexpr inline Rect& operator-=(const Size<T>& rhs) {
         width -= rhs.width;
         height -= rhs.height;
         return *this;
      }

      constexpr inline Rect& operator*=(const Size<T>& rhs) {
         width *= rhs.width;
         height *= rhs.height;
         return *this;
      }

      constexpr inline Rect& operator/=(const Size<T>& rhs) {
         width /= rhs.width;
         height /= rhs.height;
         return *this;
      }

      constexpr inline Rect operator+(const Rect<T>& rhs) const {
         Rect<T> val = *this;
         val.x += rhs.x;
         val.y += rhs.y;
         val.width += rhs.width;
         val.height += rhs.height;
         return val;
      }

      constexpr inline Rect operator-(const Rect<T>& rhs) const {
         Rect<T> val = *this;
         val.x -= rhs.x;
         val.y -= rhs.y;
         val.width -= rhs.width;
         val.height -= rhs.height;
         return val;
      }

      constexpr inline Rect& operator+=(const Rect<T>& rhs) {
         x += rhs.x;
         y += rhs.y;
         width += rhs.width;
         height += rhs.height;
         return *this;
      }

      constexpr inline Rect& operator-=(const Rect<T>& rhs) {
         x -= rhs.x;
         y -= rhs.y;
         width -= rhs.width;
         height -= rhs.height;
         return *this;
      }

      constexpr inline Rect& operator*=(const Rect<T>& rhs) {
         x *= rhs.x;
         y *= rhs.y;
         width *= rhs.width;
         height *= rhs.height;
         return *this;
      }

      constexpr inline Rect& operator/=(const Rect<T>& rhs) {
         x /= rhs.x;
         y /= rhs.y;
         width /= rhs.width;
         height /= rhs.height;
         return *this;
      }

      constexpr inline bool isInverted() const {return (width < 0 || height < 0);}
      constexpr inline Rect& centerOnPoint(const Pos<R_FLOAT>& p) {
         return setPos(p - Pos<R_FLOAT>(size().toPos() / 2));
      } /// Return the rect such that it would be centered on point p
      constexpr inline Rect& embiggen(T amt) {
         return *this += Rect<T>(-amt, -amt, 2 * amt, 2 * amt);
      } //shrink/expand borders evenly. Perfectly cromulent name.
      constexpr inline Rect& stretchVertical(T amt) {
         return *this += Rect<T>(0, -amt, 0, 2 * amt);
      } //embiggen tallness evenly
      constexpr inline Rect& stretchHorizontal(T amt) {
         return *this += Rect<T>(-amt, 0, 2 * amt, 0);
      } //embiggen wideness evenly
      constexpr inline Rect& chopTop(T amt) {
         y += amt;
         height -= amt;
         return *this;
      } //remove the topmost amt of the rectangle and return the remainder (moves y, cuts height)
      constexpr inline Rect& chopBottom(T amt) {
         height -= amt;
         return *this;
      } //remove the bottommost amt of the rectangle and return the remainder (cuts height)
      constexpr inline Rect& chopRight(T amt) {
         width -= amt;
         return *this;
      } //remove the rightmost amt of the rectangle and return the remainder (cuts width)
      constexpr inline Rect& chopLeft(T amt) {
         x += amt;
         width -= amt;
         return *this;
      } //remove the leftmost amt of the rectangle and return the remainder (moves x, cuts width)
      constexpr inline Rect& pushX(T amt) {
         x += amt;
         return *this;
      }

      constexpr inline Rect& pushY(T amt) {
         y += amt;
         return *this;
      }

      constexpr inline Rect& stretchLeft(T amt) {
         x -= amt;
         width += amt;
         return *this;
      }

      constexpr inline Rect& stretchRight(T amt) {
         width += amt;
         return *this;
      }

      constexpr inline Rect& stretchDown(T amt) {
         height += amt;
         return *this;
      }

      constexpr inline Rect& stretchUp(T amt) {
         y -= amt;
         height += amt;
         return *this;
      }

      constexpr inline Rect& mirrorRight() {
         x += width;
         return *this;
      }

      constexpr inline Rect& mirrorLeft() {
         x -= width;
         return *this;
      }

      constexpr inline Rect& mirrorUp() {
         y -= height;
         return *this;
      }

      constexpr inline Rect& mirrorDown() {
         y += height;
         return *this;
      }

      //return the horizontal centerline of the rectangle
      [[nodiscard]] constexpr inline Line<T> centerLineH() const {
         auto c = center();
         return {0, c.y, width, c.y};
      }

      //return the vertical centerline of the rectangle
      [[nodiscard]] constexpr inline Line<T> centerLineV() const {
         auto c = center();
         return {c.x, 0, c.x, height};
      }

      [[nodiscard]] constexpr inline Rect embiggen(T amt) const {
         return *this + Rect<T>(-amt, -amt, 2 * amt, 2 * amt);
      }

      [[nodiscard]] constexpr inline Rect stretchVertical(T amt) const { return *this + Rect<T>(0, -amt, 0, 2 * amt); }

      [[nodiscard]] constexpr inline Rect stretchHorizontal(T amt) const {
         return *this + Rect<T>(-amt, 0, 2 * amt, 0);
      }

      [[nodiscard]] constexpr inline Rect chopTop(T amt) const {
         auto retval = *this;
         retval.y += amt;
         retval.height -= amt;
         return retval;
      }

      [[nodiscard]] constexpr inline Rect chopBottom(T amt) const {
         auto retval = *this;
         retval.height -= amt;
         return retval;
      }

      [[nodiscard]] constexpr inline Rect chopRight(T amt) const {
         auto retval = *this;
         retval.width -= amt;
         return retval;
      }

      [[nodiscard]] constexpr inline Rect chopLeft(T amt) const {
         auto retval = *this;
         retval.x += amt;
         retval.width -= amt;
         return retval;
      }

      [[nodiscard]] constexpr inline Rect pushX(T amt) const {
         auto retval = *this;
         retval.x += amt;
         return retval;
      }

      [[nodiscard]] constexpr inline Rect pushY(T amt) const {
         auto retval = *this;
         retval.y += amt;
         return retval;
      }
      [[nodiscard]] constexpr inline Rect stretchLeft(T amt) const {
         auto retval = *this;
         retval.x -= amt;
         retval.width += amt;
         return retval;
      }
      [[nodiscard]] constexpr inline Rect stretchRight(T amt) const {
         auto retval = *this;
         retval.width += amt;
         return retval;
      }
      [[nodiscard]] constexpr inline Rect stretchDown(T amt) const {
         auto retval = *this;
         retval.height += amt;
         return retval;
      }
      [[nodiscard]] constexpr inline Rect stretchUp(T amt) const {
         auto retval = *this;
         retval.y -= amt;
         retval.height += amt;
         return retval;
      }
      [[nodiscard]] constexpr inline Rect mirrorRight() const {
         auto retval = *this;
         retval.x += width;
         return retval;
      }
      [[nodiscard]] constexpr inline Rect mirrorLeft() const {
         auto retval = *this;
         retval.x -= width;
         return retval;
      }
      [[nodiscard]] constexpr inline Rect mirrorUp() const {
         auto retval = *this;
         retval.y -= height;
         return retval;
      }
      [[nodiscard]] constexpr inline Rect mirrorDown() const {
         auto retval = *this;
         retval.y += height;
         return retval;
      }
      [[nodiscard]] constexpr inline bool containsX(const Vec2<T>& point) const {
         return (point.x >= x && point.x < x + width);
      }
      [[nodiscard]] constexpr inline bool containsY(const Vec2<T>& point) const {
         return (point.y >= y && point.y < y + height);
      }
      [[nodiscard]] constexpr inline bool contains(const Rect<T>& other) const {
         return other.x >= x && other.y >= y && other.width <= width && other.height <= height;
      }
      [[nodiscard]] constexpr inline bool contains(const Vec2<T>& point) const {
         return containsX(point) && containsY(point);
      }

      ///Combines two rectangles into one large rectangle, top left corner of top left rectangle and bottom right corner of bottom right rectangle
      [[nodiscard]] constexpr inline Rect combine(const Rect<T>& other) const {
         T minX = Math::min(x, other.x);
         T minY = Math::min(y, other.y);
         T maxX = Math::max(x + width, other.x + other.width);
         T maxY = Math::max(y + height, other.y + other.height);
         return Rect<T>(minX, minY, maxX - minX, maxY - minY);
      }

      constexpr inline void clampWidth(const Vec2<T>& widthRange) {
         if (width < widthRange.x) width = widthRange.x;
         if (width > widthRange.y) width = widthRange.y;
      }

      constexpr inline void clampHeight(const Vec2<T>& heightRange) {
         if (height < heightRange.x)
            height = heightRange.x;
         if (height > heightRange.y) height = heightRange.y;
      }

      // constrains this rectangle's position so that it is INSIDE the larger rectangle, otherwise does nothing
      constexpr inline Rect& restrictTo(const Rect& larger) {
         if (x + width > larger.x + larger.width) x = larger.x + larger.width - width;
         if (x < larger.x) x += larger.x - x;
         if (y + height > larger.y + larger.height) y = larger.y + larger.height - height;
         if (y < larger.y) y += larger.y - y;
         return *this;
      }

      [[nodiscard]] constexpr inline Pos<T> topLeft() const { return {x, y}; }
      [[nodiscard]] constexpr inline Pos<T> topRight() const { return {x + width, y}; }
      [[nodiscard]] constexpr inline Pos<T> bottomRight() const { return {x + width, y + height}; }
      [[nodiscard]] constexpr inline Pos<T> bottomLeft() const { return {x, y + height}; }
      [[nodiscard]] constexpr inline Line<T> left() const { return {topLeft(), bottomLeft()}; }
      [[nodiscard]] constexpr inline Line<T> right() const { return {topRight(), bottomRight()}; }
      [[nodiscard]] constexpr inline Line<T> top() const { return {topLeft(), topRight()}; }
      [[nodiscard]] constexpr inline Line<T> bottom() const { return {bottomLeft(), bottomRight()}; }
      [[nodiscard]] constexpr inline Line<T> backSlash() const { return {topLeft(), bottomRight()}; }
      [[nodiscard]] constexpr inline Line<T> frontSlash() const { return {bottomLeft(), topRight()}; }
      // return a rectangle that would render the same but has positive width and height
      constexpr inline void normalize() {
         if (width < 0) {
            x += width;
            width = -width;
         }
         if (height < 0) {
            y += height;
            height = -height;
         }
      }

      [[nodiscard]] constexpr inline Rect normalized() const {
         auto retval = Rect(*this);
         retval.normalize();
         return retval;
      }

      [[nodiscard]] constexpr inline bool collides(const Rect& other) const {
         return ((x < (other.x + other.width) && (x + width) > other.x) &&
                 (y < (other.y + other.height) && (y + height) > other.y));
      }

      ///stops at 3 because that's the same as 4
      [[nodiscard]] constexpr inline int getCollisionType(const Rect& other) const {
         int pointCount = 0;
         if (contains(other.topLeft())) pointCount++;
         if (contains(other.topRight())) pointCount++;
         if (contains(other.bottomRight())) pointCount++;
         if (pointCount == 3) return 3;
         if (contains(other.bottomLeft())) pointCount++;
         return pointCount;
      }

      [[nodiscard]] constexpr inline Rect getOverlap(const Rect& other) const {
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
         if (!collisionType) {
            collisionType = other.getCollisionType(*this);
            isSecondaryCollision = true;
         }
         auto& primaryRect = isSecondaryCollision ? other : *this;
         auto& secondaryRect = isSecondaryCollision ? *this : other;
         switch (collisionType) {
            case 0:
               //potentially a collision or not
               if (xr > xl + xlw || yb > yt + yth) return {}; //no collision
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
                  if (xprime && yprime) {
                     //need to distinguish between forms 1 and 2 which both have the top left prime point inside the rect
                     if (contains(secondaryRect.bottomLeft())) {
                        //form 2
                        horizontalForm = false;
                     }
                  } else if ((!xprime && yprime) || (!xprime && !yprime)) {
                     horizontalForm = false;
                  }
                  if (horizontalForm) return {xr, yb, xrw, yt + yth - yb};
                  return {xr, yb, xl + xlw - xr, ybh};
               }
               //otherwise fall through to 1 point form
               [[fallthrough]];
            case 1:
               //corner collision, full width/height 2 point collisions
               return {xr, yb, xl + xlw - xr, yt + yth - yb};
            default:
               //fully inside, 3 or 4 point collisions (we won't actually get 4 point but its handled here just in case
               return secondaryRect;
         }
      }

      [[nodiscard]] inline std::string toString() const { return Vec4(x, y, width, height).toString(); }

      inline static Rect<T> fromString(const std::string& s) { Vec4<T>::fromString(s); }

      friend std::ostream& operator<<(std::ostream& os, const Rect<T>& r) {
         os << r.toString();
         return os;
      }

      [[nodiscard]] constexpr inline Pos<T> center() const { return {x + width / 2, y + height / 2}; }

      [[nodiscard]] constexpr inline Pos<T> pos() const { return {x, y}; }

      [[nodiscard]] constexpr inline Size<T> size() const { return {width, height}; }

      [[nodiscard]] constexpr inline Rect<T> toSizeRect() const { return {0, 0, width, height}; }

      constexpr inline Rect& setSize(const Size<T>& size) {
         width = size.x;
         height = size.y;
         return *this;
      }

      constexpr inline Rect& setHeight(T size) {
         height = size;
         return *this;
      }

      constexpr inline Rect& setWidth(T size) {
         width = size;
         return *this;
      }

      constexpr inline Rect& setPos(const Pos<T>& pos) {
         x = pos.x;
         y = pos.y;
         return *this;
      }

      //return the smallest rect that contains both rects a and b
      [[nodiscard]] constexpr inline Rect getBoundingRect(const Rect& other) const {
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

      static constexpr inline Rect getBoundingRect(const Rect& a, const Rect& b) {
         return a.getBoundingRect(b);
      }

      //Subrects
      // A subrect is a smaller rectangle that makes up a part of a larger rectangle. Think spritesheets.

      //returns the 'index' of a subrect, as if it were read left-to-right, top-to-bottom
      // ex.
      //     0 1 2 3
      // 0  |- - - |=|  <-- subrect {3,0}
      // 1  |- -|=|- |  <-- subrect {2,1}
      // 2  |=|- - - |  <-- subrect {0,2}
      // 3  |-|=|- - |  <-- subrect {1,3}
      [[nodiscard]] constexpr inline int getSubRectIndex(const Size<R_FLOAT>& size, const Pos<R_FLOAT>& pos) const {
         auto coord = getSubRectCoord(size, pos);
         auto columnCount = width / size.x;
         return coord.y * columnCount + coord.x;
      }

      //Get the sub-rectangle (of size Size) that contains pos Pos. Think tilemaps.
      [[nodiscard]] constexpr inline Rect getSubRectAtPos(const Size<R_FLOAT>& size, const Pos<R_FLOAT>& pos) const {
         auto subx = pos.x / size.x;
         auto suby = pos.y / size.y;
         return Rect(subx * size.x, suby * size.y, size.x, size.y);
      }

      //Get the sub-rectangle (of size Size) at SubRectCoords coords.
      [[nodiscard]] constexpr inline Rect
      getSubRectAtCoords(const Size<R_FLOAT>& size, const SubRectCoords& coords) const {
         return Rect(coords.x * size.x, coords.y * size.y, size.x, size.y);
      }

      //returns the coordinates of the above subrect in grid-form (ie the 3rd subrect from the left would be {3,0}
      [[nodiscard]] constexpr inline SubRectCoords
      getSubRectCoord(const Size<R_FLOAT>& size, const Pos<R_FLOAT>& pos) const {
         //divide by 0?
         auto subx = pos.x / size.x;
         auto suby = pos.y / size.y;
         return {(int) subx, (int) suby};
      }

      //get an actual subrect given a subrect size and an index
      [[nodiscard]] constexpr inline Rect getSubRect(const Size<R_FLOAT>& size, int index) const {
         int columnCount = width / size.x;
         int coordY = index / columnCount;
         int coordX = index % columnCount;
         R_FLOAT posX = (float) coordX * size.x;
         R_FLOAT posY = (float) coordY * size.y;
         return Rect(posX, posY, size.x, size.y);
      }

      //get the rectangle that contains the subrects at start and stop indices (as topleft and bottom right respectively)
      [[nodiscard]] constexpr inline Rect getSubRect(const Size<R_FLOAT>& size, int indexStart, int indexStop) const {
         auto a = getSubRect(size, indexStart);
         auto b = getSubRect(size, indexStop);
         return getBoundingRect(a, b);
      }

      [[nodiscard]] Circle circumscribe() const;

      [[nodiscard]] Circle inscribe() const;

      constexpr inline void clear() {
         x = 0, y = 0, width = 0;
         height = 0;
      }

      constexpr inline std::array<Vec2<R_FLOAT>, 4> transform(const Matrix& m) const {
         std::array<Vec2<R_FLOAT>, 4> corners;
         corners[0] = topLeft().transform(m);
         corners[1] = topRight().transform(m);
         corners[2] = bottomRight().transform(m);
         corners[3] = bottomLeft().transform(m);
         return corners;
      }


   private:
      // Private implementation function with a templated bool parameter
      template<bool isVertical>
      [[nodiscard]] std::vector<Rect<T>> splitImpl(const std::vector<Percent>& percentages) const {
         std::vector<Rect<T>> result;

         // If no percentages provided, split into two equal parts
         if (percentages.empty()) {
            result.reserve(2);
            Rect<T> firstHalf = *this;
            Rect<T> secondHalf = *this;

            if (isVertical) {
               // Vertical split in half
               firstHalf.height = height / 2;

               secondHalf.height = height / 2;
               secondHalf.y = y + height / 2;
            } else {
               // Horizontal split in half
               firstHalf.width = width / 2;

               secondHalf.width = width / 2;
               secondHalf.x = x + width / 2;
            }

            result.push_back(firstHalf);
            result.push_back(secondHalf);
            return result;
         }

         // Normal case with percentages provided
         result.reserve(percentages.size() + 1);

         // Calculate the total percentage from the input vector
         double totalPercentage = 0.0;
         for (const auto& percent: percentages) {
            totalPercentage += percent.get();
         }

         // Calculate the remaining percentage for the last rectangle
         double remainingPercentage = 100.0 - totalPercentage;

         // Make sure the total doesn't exceed 100%
         if (remainingPercentage < -1e-6) {
            // Handle error - return a reasonable fallback (split in half)
            result.clear();
            return splitImpl<isVertical>(std::vector<Percent>());
         }

         // The dimension we're splitting (width for horizontal, height for vertical)
         T totalDimension = isVertical ? height : width;

         // Current position offset
         T currentPos = isVertical ? y : x;

         // Create rectangles for each percentage in the vector
         for (const auto& percent: percentages) {
            Rect<T> newRect = *this;

            // Calculate the new dimension based on the percentage
            T newDimension = static_cast<T>(totalDimension * (percent.get() / 100.0));

            if (isVertical) {
               newRect.height = newDimension;
               newRect.y = currentPos;
               currentPos += newDimension;
            } else {
               newRect.width = newDimension;
               newRect.x = currentPos;
               currentPos += newDimension;
            }

            result.push_back(newRect);
         }

         // Create the final (N+1)th rectangle with the remaining percentage, if applicable
         if (remainingPercentage != 0) {
            Rect<T> lastRect = *this;
            T lastDimension = static_cast<T>(totalDimension * (std::max(0.0, remainingPercentage) / 100.0));

            if (isVertical) {
               lastRect.height = lastDimension;
               lastRect.y = currentPos;
            } else {
               lastRect.width = lastDimension;
               lastRect.x = currentPos;
            }

            result.push_back(lastRect);
         }

         return result;
      }

      // Helper function to create a tuple from a vector
      template<std::size_t... I>
      auto make_tuple_from_vector(const std::vector<Rect<T>>& vec, std::index_sequence<I...>) const {
         return std::make_tuple(vec[I]...);
      }

   public:
      // Unified splitH function that handles all cases correctly
      template<bool AsTuple = false>
      [[nodiscard]] auto splitH() const {
         std::vector<Rect<T>> resultVec = splitImpl<false>(std::vector<Percent>{});

         if constexpr (AsTuple) {
            return make_tuple_from_vector(resultVec, std::make_index_sequence<2>{});
         } else {
            return resultVec;
         }
      }

      // Overload for a single Percent value
      template<bool AsTuple = false>
      [[nodiscard]] auto splitH(const Percent& percent) const {
         std::vector<Percent> percentages{percent};
         std::vector<Rect<T>> resultVec = splitImpl<false>(percentages);

         if constexpr (AsTuple) {
            return make_tuple_from_vector(resultVec, std::make_index_sequence<2>{});
         } else {
            return resultVec;
         }
      }

      [[nodiscard]] std::vector<Rect<T>>
      splitH(const std::vector<Percent>& percentages) const { return splitImpl<false>(percentages); }

      template<bool AsTuple = false, typename... Args, typename = std::enable_if_t<(sizeof...(Args) > 1)>>
      [[nodiscard]] auto splitH(const Args& ... args) const {
         std::vector<Percent> percentages;
         percentages.reserve(sizeof...(args));
         (percentages.push_back(Percent(static_cast<double>(args))), ...);
         std::vector<Rect<T>> resultVec = splitImpl<false>(percentages);
         if constexpr (AsTuple) {
            return make_tuple_from_vector(resultVec, std::make_index_sequence<sizeof...(Args) + 1>{});
         } else {
            return resultVec;
         }
      }

      template<bool AsTuple = false>
      [[nodiscard]] auto splitV() const {
         std::vector<Rect<T>> resultVec = splitImpl<true>(std::vector<Percent>{});
         if constexpr (AsTuple) {
            return make_tuple_from_vector(resultVec, std::make_index_sequence<2>{});
         } else {
            return resultVec;
         }
      }

      template<bool AsTuple = false>
      [[nodiscard]] auto splitV(const Percent& percent) const {
         std::vector<Percent> percentages{percent};
         std::vector<Rect<T>> resultVec = splitImpl<true>(percentages);
         if constexpr (AsTuple) {
            return make_tuple_from_vector(resultVec, std::make_index_sequence<2>{});
         } else {
            return resultVec;
         }
      }

      [[nodiscard]] std::vector<Rect<T>> splitV(const std::vector<Percent>& percentages) const {
         return splitImpl<true>(percentages);
      }

      /// Takes a point p relative to this rect and returns an array that tells us what fraction
      /// of the rectangles size that point represents. ie the dead center fo the rectangle would represent
      /// {0.5, 0.5}. Not quite the same as UV because it uses the typical bottom-right convention and not the
      /// top-right convention that UV uses.
      /// To avoid making things ugly, returns 0,0 if x or y is 0;
      [[nodiscard]] Vec2<Fraction> ratio(const Pos<float>& p) const {
         if (width == 0 || height == 0) return {0, 0};
         return {p.x / width, p.y / height};
      }


      [[nodiscard]] Pos<T> scalePoint(const Vec2<Fraction>& p_f, const Rect& r) const {
         auto retval = Pos<T>(p_f.x.get() * r.width, p_f.y.get() * r.height) + r.pos();
         return retval;
      }

      /// Takes point p relative to this rect and returns the same point relative to rect r
      /// ie if point p is in the dead center of this rectangle then the return value will
      /// be the point that is the dead center of rect r
      [[nodiscard]] Pos<T> scalePoint(const Pos<T>& p, const Rect& r) const {
         return scalePoint(ratio(p), r);
      }

      template<bool AsTuple = false, typename... Args, typename = std::enable_if_t<(sizeof...(Args) > 1)>>
      [[nodiscard]] auto splitV(const Args& ... args) const {
         std::vector<Percent> percentages;
         percentages.reserve(sizeof...(args));
         (percentages.push_back(Percent(static_cast<double>(args))), ...);
         std::vector<Rect<T>> resultVec = splitImpl<true>(percentages);
         if constexpr (AsTuple) {
            return make_tuple_from_vector(resultVec, std::make_index_sequence<sizeof...(Args) + 1>{});
         } else {
            return resultVec;
         }
      }

      [[nodiscard]] constexpr Rect alignRight(const Pos<T>& p){ return {p.x - width, y, width, height}; }
      [[nodiscard]] constexpr Rect alignLeft(const Pos<T>& p){ return {p.x, y, width, height}; }
      [[nodiscard]] constexpr Rect alignTop(const Pos<T>& p){ return {x, p.y, width, height}; }
      [[nodiscard]] constexpr Rect alignBottom(const Pos<T>& p){ return {x, p.y - height, width, height}; }
      [[nodiscard]] constexpr Rect centerH(const Pos<T>& p){ return {p.x - width / 2, y, width, height}; }
      [[nodiscard]] constexpr Rect centerV(const Pos<T>& p){ return {x, p.y - height / 2, width, height}; }

      //centers text in the given rectangle
      [[nodiscard]] static Rect<float> textRectangleCentered(const std::string& text, const Pos<float>& pos, const ReyEngineFont& font);

      T x;
      T y;
      T width;
      T height;
   };

   struct Circle{
      constexpr inline Circle(const Pos<R_FLOAT>& center, R_FLOAT radius): center(center), radius(radius){}
      constexpr inline Circle(const Circle& rhs): center(rhs.center), radius(rhs.radius){}
      /// create the circle that comprises the three points
      static inline std::optional<Circle> fromPoints(const Pos<R_FLOAT>& a, const Pos<R_FLOAT>& b, const Pos<R_FLOAT>& c){
         // Convert input points to doubles for precise calculation
         R_FLOAT x1 = a.x, y1 = a.y;
         R_FLOAT x2 = b.x, y2 = b.y;
         R_FLOAT x3 = c.x, y3 = c.y;

         // Calculate the perpendicular bisector of two chords
         R_FLOAT ux = 2 * (x2 - x1);
         R_FLOAT uy = 2 * (y2 - y1);
         R_FLOAT vx = 2 * (x3 - x1);
         R_FLOAT vy = 2 * (y3 - y1);
         R_FLOAT u = (x2*x2 - x1*x1 + y2*y2 - y1*y1);
         R_FLOAT v = (x3*x3 - x1*x1 + y3*y3 - y1*y1);

         // Calculate determinant
         R_FLOAT det = ux * vy - uy * vx;

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
      [[nodiscard]] inline Pos<R_FLOAT> getPoint(Radians r) const {return {center.x + radius * (float)std::cos(r.get()), center.y + radius * (float)std::sin(r.get())};}
      /// Return the angular offset from the right-handed horizontal that corresponds to the given point.
      ///
      /// \param pos: A point along a normal
      /// \return
      [[nodiscard]] inline Radians getRadians(const Pos<R_FLOAT>& pos) const {
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
      [[nodiscard]] inline Pos<R_FLOAT> getTangentPoint(const Pos<R_FLOAT>& pos) const {return getPoint(getRadians(pos));}
      /// Returns a line that is tangent to the circle at the given normal point.
      /// \param pos: a point lying on a line that is normal to the circle.
      /// \param length: the length of the tangent
      /// \return: a tangent line that intersects the given normal
      [[nodiscard]] inline Line<R_FLOAT> getTangentLine(const Pos<R_FLOAT>& pos, double length) const {
         auto point = getTangentPoint(pos);
         // Calculate the vector from center to tangent point
         R_FLOAT dx = point.x - center.x;
         R_FLOAT dy = point.y - center.y;
         // Calculate the perpendicular vector (rotate by 90 degrees)
         R_FLOAT perpX = -dy;
         R_FLOAT perpY = dx;
         // Normalize the perpendicular vector
         R_FLOAT magnitude = std::sqrt(perpX*perpX + perpY*perpY);
         perpX /= magnitude;
         perpY /= magnitude;
         // Calculate the start and end points of the tangent line
         R_FLOAT halfLength = length / 2.0;
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

//   struct CircleProperty : public Property<Circle>{
//      using Property<Circle>::operator=;
//      CircleProperty(const std::string& instanceName,  Circle&& defaultvalue=Circle({},0))
//      : Property<Circle>(instanceName, PropertyTypes::Color, std::move(defaultvalue))
//      {}
//      std::string toString() const override {
//         auto fvec = value.center.getElements();
//         fvec.push_back(value.radius);
//         return string_tools::listJoin(fvec);
//      }
//      Circle fromString(const std::string& str) override {
//         auto split = string_tools::fromList(str);
//         return {Pos<R_FLOAT>(STOF(split.at(0)), STOF(split.at(1))), STOF(split.at(2))};
//      }
//   };

   struct CircleSector : public Circle {
      static constexpr double FIRST_QUADRANT_ANGLE = 0;
      static constexpr double SECOND_QUADRANT_ANGLE = 90;
      static constexpr double THIRD_QUADRANT_ANGLE = 180;
      static constexpr double FOURTH_QUADRANT_ANGLE = 270;
      static CircleSector firstQuadrant(const Pos<R_FLOAT>& center, double radius){return {center, radius, FIRST_QUADRANT_ANGLE, FIRST_QUADRANT_ANGLE+90};}
      static CircleSector secondQuadrant(const Pos<R_FLOAT>& center, double radius){return {center, radius, SECOND_QUADRANT_ANGLE, SECOND_QUADRANT_ANGLE+90};}
      static CircleSector thirdQuadrant(const Pos<R_FLOAT>& center, double radius){return {center, radius, THIRD_QUADRANT_ANGLE, THIRD_QUADRANT_ANGLE+90};}
      static CircleSector fourthQuadrant(const Pos<R_FLOAT>& center, double radius){return {center, radius, FOURTH_QUADRANT_ANGLE, FOURTH_QUADRANT_ANGLE+90};}
      constexpr inline CircleSector(): Circle({},0), startAngle(0), endAngle(0){}
      constexpr inline CircleSector(const Pos<R_FLOAT>& center, double radius, double startAngle, double endAngle): Circle(center, radius), startAngle(startAngle), endAngle(endAngle){}
      constexpr inline CircleSector(const Circle& c, double startAngle, double endAngle): Circle(c), startAngle(startAngle), endAngle(endAngle){}
      constexpr inline CircleSector(const CircleSector& rhs): Circle(rhs), startAngle(rhs.startAngle), endAngle(rhs.endAngle){}
      inline void setDirection(const UnitVector2& newDirection){
         // Calculate the new center angle from the direction vector
         double newCenterAngle = std::atan2(newDirection.x(), newDirection.y()) * 180.0 / M_PI ;

         // Ensure the angle is in the range [0, 360)
         while (newCenterAngle < 0) newCenterAngle += 360;
         while (newCenterAngle >= 360) newCenterAngle -= 360;

         // Calculate the current arc size
         double arcSize = endAngle - startAngle;

         // Calculate new start and end angles
         // Place the new center angle in the middle of the arc
         startAngle = newCenterAngle - arcSize / 2.0;
         endAngle = newCenterAngle + arcSize / 2.0;
      }
      /// Return the angle at pct percent of the arc from "left" to "right"
      inline Degrees getPctOfArc(const Percent& pct){
         return ((endAngle - startAngle) * pct.get()/100) + startAngle;
      }
      Degrees arcDegrees(){return endAngle - startAngle;}
      inline UnitVector2 direction(){
         // Calculate the center angle in degrees
         double centerAngle = startAngle + (endAngle - startAngle) / 2.0;
         // Convert to radians
         double angleRadians = centerAngle * M_PI / 180.0;
         // Calculate the direction vector components
         R_FLOAT dirX = std::cos(angleRadians);
         R_FLOAT dirY = std::sin(angleRadians);
         return {dirX, dirY};
      }
      double startAngle; //degrees
      double endAngle; //degrees
   };

   //should maybe use eigen transforms. one day.
//should maybe use eigen transforms. one day.
   template <uint8_t D>
   requires (D==2 || D==3)
   struct Transform {
      Transform(){matrix = MatrixIdentity();}
      Transform(const Matrix& m){matrix = m;}
      Transform& operator*=(const Transform& rhs){matrix = MatrixMultiply(matrix, rhs.matrix);return *this;}
      Transform& operator=(const Matrix& m){matrix = m;}
      Matrix matrix;

      // Base implementation
      [[nodiscard]] Matrix inverse() const {return MatrixInvert(matrix);}
   };

// Specialization for 2D transforms
   struct Transform2D : public Transform<2> {
      Transform2D(): Transform<2>(){}
      Transform2D(const Matrix& m): Transform<2>(m){}
      Transform2D& operator*=(const Transform2D& rhs){matrix = MatrixMultiply(matrix, rhs.matrix);return *this;}
      Transform2D operator*(const Transform2D& rhs){return MatrixMultiply(matrix, rhs.matrix);}
      // Enhanced 2D transform function with more arguments
      [[nodiscard]] inline Vec2<float> transform(const Vec2<float>& point,
                                                 bool applyScale = true,
                                                 bool applyRotation = true,
                                                 bool applyTranslation = true) const {
         // Create a copy of the matrix to selectively apply transformations
         Matrix workingMatrix = matrix;

         if (!applyScale) {
            // Remove scaling from the working matrix
            Vec2<float> scale = extractScale();
            if (scale.x != 0 && scale.y != 0) {
               workingMatrix = MatrixMultiply(MatrixScale(1.0f/scale.x, 1.0f/scale.y, 1.0f), workingMatrix);
            }
         }

         if (!applyRotation) {
            // Remove rotation from the working matrix
            float rotation = extractRotation();
            workingMatrix = MatrixMultiply(MatrixRotate({0, 0, 1}, -rotation), workingMatrix);
         }

         if (!applyTranslation) {
            // Remove translation from the working matrix
            Vec2<float> translation = extractTranslation();
            workingMatrix = MatrixMultiply(MatrixTranslate(-translation.x, -translation.y, 0), workingMatrix);
         }

         // Use the modified matrix to transform the point
         Vector3 vec3Point = {point.x, point.y, 0.0f};
         Vector3 transformed = Vector3Transform(vec3Point, workingMatrix);
         return Vec2<float>{transformed.x, transformed.y};
      }

      // Additional utility functions for 2D
      [[nodiscard]] inline Vec2<float> extractTranslation() const {
         return Vec2<float>{matrix.m12, matrix.m13};
      }

      [[nodiscard]] inline float extractRotation() const {
         // Extract rotation angle from the matrix
         return atan2f(matrix.m1, matrix.m0);
      }

      [[nodiscard]] inline Vec2<float> extractScale() const {
         // Extract scale from the matrix
         float scaleX = Vector2Length({matrix.m0, matrix.m4});
         float scaleY = Vector2Length({matrix.m1, matrix.m5});
         return Vec2<float>{scaleX, scaleY};
      }

      [[nodiscard]] inline Transform2D inverse() const {return MatrixInvert(matrix);}

      // Enhanced 2D rotate with optional center point
      void inline rotate(const Radians& r, const Vec2<float>& centerPoint = {0.0f, 0.0f}) {
         if (centerPoint.x == 0.0f && centerPoint.y == 0.0f) {
            // Simple rotation around origin
            matrix = MatrixMultiply(MatrixRotate({0, 0, 1}, (float)r.get()), matrix);
         } else {
            // Rotation around a specific point:
            // 1. Translate center to origin
            translate(-centerPoint);
            // 2. Rotate
            matrix = MatrixMultiply(MatrixRotate({0, 0, 1}, (float)r.get()), matrix);
            // 3. Translate back
            translate(centerPoint);
         }
      }

      // Enhanced 2D translate with optional scale factor
      void inline translate(const Vec2<float>& vec, float scaleFactor = 1.0f) {
         matrix = MatrixMultiply(MatrixTranslate(vec.x * scaleFactor, vec.y * scaleFactor, 0), matrix);
      }

      // Enhanced 2D translate with optional scale factor
      void inline setPosition(const Vec2<float>& vec) {
         matrix.m12 = vec.x;
         matrix.m13 = vec.y;
      }

      // Enhanced 2D scale with optional center point
      void inline scale(const Vec2<float>& scale, const Vec2<float>& centerPoint = {0.0f, 0.0f}) {
         if (centerPoint.x == 0.0f && centerPoint.y == 0.0f) {
            // Simple scaling around origin
            matrix = MatrixMultiply(MatrixScale(scale.x, scale.y, 1), matrix);
         } else {
            // Scaling around a specific point:
            // 1. Translate center to origin
            translate(-centerPoint);
            // 2. Scale
            matrix = MatrixMultiply(MatrixScale(scale.x, scale.y, 1), matrix);
            // 3. Translate back
            translate(centerPoint);
         }
      }

      // New function: shear transformation
      void inline shear(float shearX, float shearY) {
         Matrix shearMatrix = {
               1.0f, shearY, 0.0f, 0.0f,
               shearX, 1.0f, 0.0f, 0.0f,
               0.0f, 0.0f, 1.0f, 0.0f,
               0.0f, 0.0f, 0.0f, 1.0f
         };
         matrix = MatrixMultiply(shearMatrix, matrix);
      }

      // New function: reflect across a line defined by a point and angle
      void inline reflect(const Vec2<float>& point, const Radians& angle) {
         // 1. Translate to origin
         translate(-point);
         // 2. Rotate to align with axis
         Radians negAngle = Radians(-(float)angle.get());
         rotate(negAngle);
         // 3. Reflect across x-axis (flip y)
         matrix = MatrixMultiply(MatrixScale(1.0f, -1.0f, 1.0f), matrix);
         // 4. Rotate back
         rotate(angle);
         // 5. Translate back
         translate(point);
      }

      // New function: compose with another transform with blending factor
      void inline compose(const Transform<2>& other, float blendFactor = 1.0f) {
         if (blendFactor == 1.0f) {
            matrix = MatrixMultiply(other.matrix, matrix);
         } else {
            // Linear interpolation between matrices
            // Note: This is a simplistic approach; for proper matrix interpolation,
            // you'd typically decompose, interpolate components, and recompose
            Matrix blendedMatrix = matrix;
            for (int i = 0; i < 16; i++) {
               float* m1 = &matrix.m0 + i;
               const float* m2 = &other.matrix.m0 + i;
               float* result = &blendedMatrix.m0 + i;
               *result = *m1 + blendFactor * (*m2 - *m1);
            }
            matrix = blendedMatrix;
         }
      }
      inline friend std::ostream& operator<<(std::ostream& os, const Transform2D& t) {std::cout << t.matrix << std::endl; return os;}
   };

   // Specialization for 3D transforms
   struct Transform3D : public Transform<3> {
      Matrix matrix;
      [[nodiscard]] Matrix getMatrix() const {return matrix;};

      [[nodiscard]] Vector3 transform(const Vector3& point) const {
         return Vector3Transform(point, matrix);
      }

      [[nodiscard]] Matrix inverse() const {return MatrixInvert(matrix);}

      void rotate(const Vector3& axis, float angle) {
         matrix = MatrixMultiply(MatrixRotate(axis, angle), matrix);
      }

      void translate(const Vector3& vec) {
         matrix = MatrixMultiply(MatrixTranslate(vec.x, vec.y, vec.z), matrix);
      }

      void scale(const Vector3& scale) {
         matrix = MatrixMultiply(MatrixScale(scale.x, scale.y, scale.z), matrix);
      }
   };

//   struct Transform2DProperty : public Property<Transform2D>{
//      using Property<Transform2D>::operator=;
//      Transform2DProperty(const std::string& instanceName,  Transform2D&& defaultvalue={})
//      : Property<Transform2D>(instanceName, PropertyTypes::Color, std::move(defaultvalue))
//      {}
//      std::string toString() const override {return "{}";}
//      Transform2D fromString(const std::string& str) override {return Transform2D();}
//   };

   struct ColorRGBA {
      ColorRGBA(): r(0), g(0), b(0), a(255){}
      constexpr ColorRGBA(int r, int g, int b): r(r), g(g), b(b), a(255){}
      constexpr ColorRGBA(int r, int g, int b, int a): r(r), g(g), b(b), a(a){}
      constexpr ColorRGBA(const Color& color): r(color.r), g(color.g), b(color.b), a(color.a){}
      constexpr inline ColorRGBA& operator=(const Color& rhs){r = rhs.r; g=rhs.g; b=rhs.b; a=rhs.a; return *this;}
      constexpr inline operator Color() const {return {r, g, b, a};}
      void dim(Fraction p){a = (float)a * Fraction(p).get();}
      inline static ColorRGBA random(int alpha = -1){
         auto retval = ColorRGBA(std::rand() % 255, std::rand() % 256, std::rand() % 256, alpha >= 0 ? alpha % 256 : std::rand() % 256);
         return retval;
      }
      Vec3<int> toVec3i() const {return {r,g,b};} //0-255
      Vec4<int> toVec4i() const {return {r,g,b,a};}//0-255
      Vec3<float> toVec3f() const {return {r / (float)255.0, g / (float)255.0, b / (float)255.0};} //0.0-1.0
      Vec4<float> toVec4f() const {return {r / (float)255.0, g / (float)255.0, b / (float)255.0, a / (float)255.0};} //0.0-1.0
      Vec3<Fraction> toVec3Fraction() const {return {r / (float)255.0, g / (float)255.0, b / (float)255.0};}
      Vec4<Fraction> toVec4Fraction() const {return {r / (float)255.0, g / (float)255.0, b / (float)255.0, a / (float)255.0};}
      friend std::ostream& operator<<(std::ostream& os, const ColorRGBA& c) {os << c.toVec4i().toString(); return os;}
      float getRelativeLuminance() const {
         // Convert to linear RGB first
         auto toLinear = [](float channel) {
            float c = channel / 255.0f;
            return (c <= 0.03928f) ? c / 12.92f : std::pow((c + 0.055f) / 1.055f, 2.4f);
         };
         float _r = toLinear(r);
         float _g = toLinear(g);
         float _b = toLinear(b);
         // WCAG formula for relative luminance
         return 0.2126f * _r + 0.7152f * _g + 0.0722f * _b;
      }
      static float getContrastRatio(const ColorRGBA& c1, const ColorRGBA& c2) {
         float L1 = c1.getRelativeLuminance();
         float L2 = c2.getRelativeLuminance();
         float lighter = std::max(L1, L2);
         float darker = std::min(L1, L2);
         return (lighter + 0.05f) / (darker + 0.05f);
      }

      ColorRGBA getReadableTextColor() const {
         ColorRGBA white(255, 255, 255, 255);
         ColorRGBA black(0, 0, 0, 255);
         float whiteContrast = getContrastRatio(*this, white);
         float blackContrast = getContrastRatio(*this, black);
         return (whiteContrast > blackContrast) ? white : black;
      }
      unsigned char r;
      unsigned char g;
      unsigned char b;
      unsigned char a;
   };


#define COLORS Colors
   namespace Colors{
      static constexpr ColorRGBA gray = {130, 130, 130, 255};
      static constexpr ColorRGBA disabledGray = {230, 230, 230, 255};
      static constexpr ColorRGBA activeBlue = {32, 155, 238, 255};
      static constexpr ColorRGBA lightGray = {200, 200, 200, 255};
      static constexpr ColorRGBA red = {230, 41, 55, 255};
      static constexpr ColorRGBA green = { 0, 228, 48, 255};
      static constexpr ColorRGBA blue = { 0, 121, 241, 255};
      static constexpr ColorRGBA black = { 0, 0, 0, 255};
      static constexpr ColorRGBA yellow = {253, 249, 0, 255};
      static constexpr ColorRGBA orange = {255, 165, 0, 255};
      static constexpr ColorRGBA purple = {127, 0, 127, 255};
      static constexpr ColorRGBA tan = {186, 129, 8, 255};
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
      ReyEngineFont(): ReyEngineFont("", 20){}
      ReyEngineFont(int fontSize): ReyEngineFont("", fontSize){}
      ReyEngineFont(const std::string& fontFile, int fontSize=20);
      ReyEngineFont(const ReyEngineFont&);
      ReyEngineFont& operator=(const ReyEngineFont&);
      ReyEngineFont& operator=(ReyEngineFont&& rhs) noexcept ;
      ~ReyEngineFont();
      Font font;
      float size = 20;
      R_FLOAT spacing = 1;
      bool isDefault = true;
      ColorRGBA color = COLORS::black;
      std::string fileName;
      ReyEngineFont copy() const;
      [[nodiscard]] Size<R_FLOAT> measure(const std::string& text) const;
      static constexpr std::string_view DEFAULT_FONT_FILE = "RobotoFlex.ttf";
   };

   struct ReyTexture;
   struct LazyTexture;
   struct ReyImage{
      ReyImage() = default;
      ReyImage(const ReyImage&) = delete;
      ReyImage& operator=(ReyImage& other) = delete;
      ReyImage(ReyImage&& other) noexcept {*this = std::move(other);};
      ReyImage& operator=(ReyImage&& other){
         std::swap(_image, other._image);
         std::swap(_imageLoaded, other._imageLoaded);
         return *this;
      }
      inline ReyImage(const Image& im){
         _image = im;
         _imageLoaded = _image.data != nullptr;
         if (!_imageLoaded) Logger::warn() << "Null image was not loaded!" << std::endl;
      }
      inline ReyImage(const char *fileType, const unsigned char *fileData, int dataSize){
         _image = LoadImageFromMemory(fileType, fileData, dataSize);
         _imageLoaded = _image.data != nullptr;
         if (!_imageLoaded) Logger::warn() << "Null image was not loaded!" << std::endl;
      }
      inline ReyImage(const ReyEngine::FileSystem::File& file){
         if (file.exists() && file.isRegularFile()) {
            _image = LoadImage(file.canonical().c_str());
         }
         _imageLoaded = _image.data != nullptr;
         if (!_imageLoaded) Logger::warn() << "Null image was not loaded!" << std::endl;
      }
      ~ReyImage(){
         release();
      }
      [[nodiscard]] void* getData() const {return _image.data;}
      operator bool() const {return _imageLoaded;}
      void release(){
         if (_imageLoaded) {
            UnloadImage(_image);
            _imageLoaded = false;
         }
      }
      [[nodiscard]] Size<float> size() const {return {(float)_image.width, (float)_image.height};}
      [[nodiscard]] ReyImage copy() const {return {ImageCopy(_image)};}
      [[nodiscard]] Image& getImage(){return _image;}
      [[nodiscard]] const Image& getImage() const {return _image;}
   protected:
      bool _imageLoaded = false;
   private:
      Image _image;
      friend class ReyTexture;
      friend class LazyTexture;
   };

   struct ReyTexture{
      ReyTexture(){}
      ReyTexture(const Size<int>& size, const ColorRGBA&);
      explicit ReyTexture(const ReyImage&);
      ReyTexture(const FileSystem::File&);
      ReyTexture(ReyTexture&& other) noexcept
      : _tex(other._tex)
      , _texLoaded(other._texLoaded)
      {
         _texLoaded = _tex.id != 0;
      }
      ReyTexture& operator=(ReyTexture&& other) noexcept {
         std::swap(_tex, other._tex);
         std::swap(_texLoaded, other._texLoaded);
         return *this;
      }
      ReyTexture& operator=(const Image& other) {
         _release();
         _tex = LoadTextureFromImage(other);
         _texLoaded = _tex.id != 0;
         return *this;
      }
      ReyTexture& operator=(const ReyImage& other) {
         _release();
         _tex = LoadTextureFromImage(other._image);
         _texLoaded = _tex.id != 0;
         return *this;
      }
//      ReyTexture& operator=(ReyImage&&); // this is ok because ReyImage is managed
      void loadTexture(const FileSystem::File& file);
      ~ReyTexture(){
         _release();
      }
      [[nodiscard]] const Texture2D& getTexture() const {return _tex;}
      [[nodiscard]] operator bool() const {return _texLoaded;}
      Size<int> size() const {return {_tex.width, _tex.height};}
   protected:
      void _release(){if (_texLoaded) UnloadTexture(_tex);}
      Texture2D _tex = {0};
      bool _texLoaded = false;
   };

   /// Since loading textures in alternate threads is not allowed, a lazy texture stores data
   /// as an image and then can be made to load it later from the main thread
   template<typename F, typename... Args>
   concept Callable = requires(F f, Args... args) {
      f(args...);
   };
   struct LazyTexture {
      LazyTexture() = default;
      LazyTexture(const LazyTexture&) = delete;
      LazyTexture(LazyTexture&& other){*this = std::move(other);}
      LazyTexture& operator=(const LazyTexture&) = delete;
      LazyTexture& operator=(LazyTexture&& other) noexcept {
         std::swap(_img, other._img);
         std::swap(_tex, other._tex);
         return *this;
      }
      void loadImage(const FileSystem::File& file){_img = LoadImage(file.canonical().c_str());}
      [[nodiscard]] bool texReady() const {return _tex && *_tex;}
      [[nodiscard]] bool imageReady() const {return _img;}
      [[nodiscard]] std::shared_ptr<ReyTexture>& getTexture() {return _tex;}
      [[nodiscard]] const ReyImage& getImage() const {return _img;}
      [[nodiscard]] bool needsConvert() const {return imageReady() && !texReady();}
      const std::shared_ptr<ReyTexture>& tryMakeTexture(){if (imageReady()) _tex = std::make_shared<ReyTexture>(_img); return _tex;}
      void releaseImage(){_img.release();}
      template<typename... Args>
      void load(Args&&... args) {
         _tex.reset();
         if constexpr (sizeof...(args) == 1) {
            using FirstArgType = std::decay_t<std::tuple_element_t<0, std::tuple<Args...>>>;
            if constexpr (std::is_same_v<FirstArgType, ReyEngine::FileSystem::File>) {
               auto& file = std::get<0>(std::forward_as_tuple(args...));
               _img = ReyImage(LoadImage(file.canonical().c_str()));
            } else {
               static_assert(sizeof...(Args) != sizeof...(Args), "No matching load function found");
            }
         } else if constexpr (Callable<decltype(LoadImage), Args...>) {
            _img = LoadImage(std::forward<Args>(args)...);
         } else if constexpr (Callable<decltype(LoadImageRaw), Args...>) {
            _img = LoadImageRaw(std::forward<Args>(args)...);
         } else if constexpr (Callable<decltype(LoadImageAnim), Args...>) {
            _img = LoadImageAnim(std::forward<Args>(args)...);
         } else if constexpr (Callable<decltype(LoadImageFromMemory), Args...>) {
            _img = LoadImageFromMemory(std::forward<Args>(args)...);
         } else if constexpr (Callable<decltype(LoadImageFromTexture), Args...>) {
            _img = LoadImageFromTexture(std::forward<Args>(args)...);
         } else if constexpr (Callable<decltype(LoadImageFromScreen), Args...>) {
            _img = LoadImageFromScreen(std::forward<Args>(args)...);
         } else {
            static_assert(sizeof...(Args) != sizeof...(Args), "No matching load function found");
         }
      }
   private:
      ReyImage _img;
      std::shared_ptr<ReyTexture> _tex;
   };

   //Underlying RenderTexture2D is different from ReyTexture's underlying Texture2D. So these are not interchangeable.
   // Use this when you are drawing to a texture.
   class RenderTarget{
   public:
      explicit RenderTarget();
      RenderTarget(const Size<int>& size);
      ~RenderTarget();
      RenderTarget& operator=(const RenderTarget&) = delete;
      void setSize(const Size<int>& newSize);
      inline Size<int> getSize() const {return _size;}
      inline void beginRenderMode(){BeginTextureMode(_tex);}
      inline void endRenderMode(){EndTextureMode();}
      inline bool ready() const {return _texLoaded;}
      [[nodiscard]] inline const Texture2D& getTexture() const {return _tex.texture;}
   protected:
      bool _texLoaded = false;
      RenderTexture2D _tex;
      Size<float> _size;
   };

   class Canvas;
   WindowSpace<Pos<R_FLOAT>> getScreenCenter();
   Size<float> getScreenSize();
   Size<float> getWindowSize();
   void setWindowSize(Size<float>);
   Pos<float> getWindowPosition();
   void setWindowPosition(Pos<float>);
   void maximizeWindow();
   void minimizeWindow();
   void drawText(const std::string& text, const Pos<R_FLOAT>& pos, const ReyEngineFont& font);
   void drawText(const std::string& text, const Pos<R_FLOAT>& pos, const ReyEngineFont& font, const ColorRGBA& color, R_FLOAT size, R_FLOAT spacing);
   void drawTextCentered(const std::string& text, const Pos<R_FLOAT>& pos, const ReyEngineFont& font);
   void drawTextCentered(const std::string& text, const Pos<R_FLOAT>& pos, const ReyEngineFont& font, const ColorRGBA& color, R_FLOAT size, R_FLOAT spacing);
   void drawTextRelative(const std::string& text, const Pos<R_FLOAT>& relPos, const ReyEngineFont& font);
   void drawText(const std::string& text, const Pos<R_FLOAT>& pos, const std::shared_ptr<ReyEngineFont>& font);
   void drawText(const std::string& text, const Pos<R_FLOAT>& pos, const std::shared_ptr<ReyEngineFont>& font, const ColorRGBA& color, R_FLOAT size, R_FLOAT spacing);
   void drawTextCentered(const std::string& text, const Pos<R_FLOAT>& pos, const std::shared_ptr<ReyEngineFont>& font);
   void drawTextCentered(const std::string& text, const Pos<R_FLOAT>& pos, const std::shared_ptr<ReyEngineFont>& font, const ColorRGBA& color, R_FLOAT size, R_FLOAT spacing);
   void drawTextRelative(const std::string& text, const Pos<R_FLOAT>& relPos, const std::shared_ptr<ReyEngineFont>& font);
   void drawTextRect(const std::string& text, const Pos<R_FLOAT>& p, const std::shared_ptr<ReyEngineFont>& font, const ColorRGBA& rectColor, float rectRoundness = 0); //draw a rectangle text centered inside it
   void drawTextRect(const std::string& text, const Rect<R_FLOAT>& r, const std::shared_ptr<ReyEngineFont>& font, const ColorRGBA& rectColor, float rectRoundness = 0); //draw a rectangle text centered inside it
   void drawRectangle(const Rect<R_FLOAT>&, const ColorRGBA& color);
   void drawRectangleRounded(const Rect<float>&, float roundness, int segments, const ColorRGBA& color);
   void drawRectangleLines(const Rect<float>&, float lineThick, const ColorRGBA& color);
   void drawRectangleRoundedLines(const Rect<float>&, float roundness, int segments, float lineThick, const ColorRGBA& color);
   void drawRectangleGradientV(const Rect<R_FLOAT>&, const ColorRGBA& color1, const ColorRGBA& color2);
   void drawRectangleGradientH(const Rect<R_FLOAT>&, const ColorRGBA& color1, const ColorRGBA& color2);
   void drawCircle(const Circle&, const ColorRGBA&  color);
   void drawCircleLines(const Circle&, const ColorRGBA&  color);
   void drawCircleSector(const CircleSector&, const ColorRGBA&  color, int segments);
   void drawCircleSectorLines(const CircleSector&, const ColorRGBA&  color, int segments);
   void drawLine(const Line<R_FLOAT>&, float lineThick, const ColorRGBA& color);
   void drawArrow(const Line<R_FLOAT>&, float lineThick, const ColorRGBA& color, float headSize=20); //Head drawn at A
   void drawArrowHead(const Line<R_FLOAT>&, float lineThick, const ColorRGBA& color, float headSize=20); //Head drawn at A
   void drawTexture(const ReyTexture& texture, const Rect<R_FLOAT>& source, const Rect<R_FLOAT>& dest, const ColorRGBA& tint);
   void drawRenderTarget(const RenderTarget&, const Pos<R_FLOAT>&, const ColorRGBA&);
   void drawRenderTargetRect(const RenderTarget&, const Rect<R_FLOAT>&, const Rect<R_FLOAT>&, const ColorRGBA&);
   inline float getFrameDelta() {return GetFrameTime();}
   inline Size<R_FLOAT> measureText(const std::string& text, const ReyEngineFont& font){return MeasureTextEx(font.font, text.c_str(), font.size, font.spacing);}
   inline Size<R_FLOAT> measureText(const std::string& text, const std::shared_ptr<ReyEngineFont>& font){return MeasureTextEx(font->font, text.c_str(), font->size, font->spacing);}
   inline void printMatrix(const Transform2D& t) { printMatrix(t.matrix);}

   /////////////////////////////////////////////////////////////////////////////////////////
   template <typename T>
   bool Pos<T>::isInside(const Rect<T>& r) {
      return r.contains(*this);
   }

   struct ScopeScissor {
      inline ScopeScissor(const Rect<R_FLOAT>& r){area = r; doScissor();}
      inline ScopeScissor(const CanvasSpace<Transform2D>& transform2D, const Rect<R_FLOAT>& r){
         auto transformedCorners = r.transform(transform2D.get().matrix);

         // Get the AABB of the transformed rectangle
         float minX = transformedCorners[0].x;
         float maxX = transformedCorners[0].x;
         float minY = transformedCorners[0].y;
         float maxY = transformedCorners[0].y;

         for(int i = 1; i < 4; i++) {
            minX = std::min(minX, transformedCorners[i].x);
            maxX = std::max(maxX, transformedCorners[i].x);
            minY = std::min(minY, transformedCorners[i].y);
            maxY = std::max(maxY, transformedCorners[i].y);
         }

         area = {minX, minY, maxX - minX, maxY - minY};
         doScissor();
      }
      inline ~ScopeScissor(){EndScissorMode();}
      inline Rect<R_FLOAT> getRect() const {return area;}
   private:
      void doScissor(){BeginScissorMode((int)area.x, (int)area.y, (int)area.width, (int)area.height);}
      Rect<R_FLOAT> area;
   };

//   constexpr auto v2_0 = Vec2<R_FLOAT>(0,1);
//   constexpr auto v2_1 = Vec2<R_FLOAT>(0,1);
//   constexpr auto v3_0 = Vec3<R_FLOAT>(2,3,4);
//   constexpr auto v3_1 = Vec3<R_FLOAT>(2,3,4);
//   constexpr auto v4 = Vec4<R_FLOAT>(5,6,7,8);
//   constexpr auto p = Pos<R_FLOAT>(9, 10);
//   constexpr auto s = Size<R_FLOAT>(11,12);
//   static_assert(Vec2<float>(0,1).x == 0);
//   static_assert(Vec2<float>(0,1).y == 1);
//   static_assert(v2_0 + v2_1 == Vec2<R_FLOAT>(0, 2));
//   static_assert(!(s != s));
//   static_assert(s == s);

}

namespace InputInterface {
   //Corresponds to ascii table
   enum class KeyCode {
      KEY_NULL = 0,
      KEY_APOSTROPHE = 39,       // Key: '
      KEY_COMMA = 44,       // Key: ,
      KEY_MINUS = 45,       // Key: -
      KEY_PERIOD = 46,       // Key: .
      KEY_SLASH = 47,       // Key: /
      KEY_ZERO = 48,       // Key: 0
      KEY_ONE = 49,       // Key: 1
      KEY_TWO = 50,       // Key: 2
      KEY_THREE = 51,       // Key: 3
      KEY_FOUR = 52,       // Key: 4
      KEY_FIVE = 53,       // Key: 5
      KEY_SIX = 54,       // Key: 6
      KEY_SEVEN = 55,       // Key: 7
      KEY_EIGHT = 56,       // Key: 8
      KEY_NINE = 57,       // Key: 9
      KEY_SEMICOLON = 59,       // Key: ;
      KEY_EQUAL = 61,       // Key: =
      KEY_A = 65,       // Key: A | a
      KEY_B = 66,       // Key: B | b
      KEY_C = 67,       // Key: C | c
      KEY_D = 68,       // Key: D | d
      KEY_E = 69,       // Key: E | e
      KEY_F = 70,       // Key: F | f
      KEY_G = 71,       // Key: G | g
      KEY_H = 72,       // Key: H | h
      KEY_I = 73,       // Key: I | i
      KEY_J = 74,       // Key: J | j
      KEY_K = 75,       // Key: K | k
      KEY_L = 76,       // Key: L | l
      KEY_M = 77,       // Key: M | m
      KEY_N = 78,       // Key: N | n
      KEY_O = 79,       // Key: O | o
      KEY_P = 80,       // Key: P | p
      KEY_Q = 81,       // Key: Q | q
      KEY_R = 82,       // Key: R | r
      KEY_S = 83,       // Key: S | s
      KEY_T = 84,       // Key: T | t
      KEY_U = 85,       // Key: U | u
      KEY_V = 86,       // Key: V | v
      KEY_W = 87,       // Key: W | w
      KEY_X = 88,       // Key: X | x
      KEY_Y = 89,       // Key: Y | y
      KEY_Z = 90,       // Key: Z | z
      KEY_LEFT_BRACKET = 91,       // Key: [
      KEY_BACKSLASH = 92,       // Key: '\'
      KEY_RIGHT_BRACKET = 93,       // Key: ]
      KEY_GRAVE = 96,       // Key: `
      // Function keys
      KEY_SPACE = 32,       // Key: Space
      KEY_ESCAPE = 256,      // Key: Esc
      KEY_ENTER = 257,      // Key: Enter
      KEY_TAB = 258,      // Key: Tab
      KEY_BACKSPACE = 259,      // Key: Backspace
      KEY_INSERT = 260,      // Key: Ins
      KEY_DELETE = 261,      // Key: Del
      KEY_RIGHT = 262,      // Key: Cursor right
      KEY_LEFT = 263,      // Key: Cursor left
      KEY_DOWN = 264,      // Key: Cursor down
      KEY_UP = 265,      // Key: Cursor up
      KEY_PAGE_UP = 266,      // Key: Page up
      KEY_PAGE_DOWN = 267,      // Key: Page down
      KEY_HOME = 268,      // Key: Home
      KEY_END = 269,      // Key: End
      KEY_CAPS_LOCK = 280,      // Key: Caps lock
      KEY_SCROLL_LOCK = 281,      // Key: Scroll down
      KEY_NUM_LOCK = 282,      // Key: Num lock
      KEY_PRINT_SCREEN = 283,      // Key: Print screen
      KEY_PAUSE = 284,      // Key: Pause
      KEY_F1 = 290,      // Key: F1
      KEY_F2 = 291,      // Key: F2
      KEY_F3 = 292,      // Key: F3
      KEY_F4 = 293,      // Key: F4
      KEY_F5 = 294,      // Key: F5
      KEY_F6 = 295,      // Key: F6
      KEY_F7 = 296,      // Key: F7
      KEY_F8 = 297,      // Key: F8
      KEY_F9 = 298,      // Key: F9
      KEY_F10 = 299,      // Key: F10
      KEY_F11 = 300,      // Key: F11
      KEY_F12 = 301,      // Key: F12
      KEY_LEFT_SHIFT = 340,      // Key: Shift left
      KEY_LEFT_CONTROL = 341,      // Key: Control left
      KEY_LEFT_ALT = 342,      // Key: Alt left
      KEY_LEFT_SUPER = 343,      // Key: Super left
      KEY_RIGHT_SHIFT = 344,      // Key: Shift right
      KEY_RIGHT_CONTROL = 345,      // Key: Control right
      KEY_RIGHT_ALT = 346,      // Key: Alt right
      KEY_RIGHT_SUPER = 347,      // Key: Super right
      KEY_KB_MENU = 348,      // Key: KB menu
      // Keypad keys
      KEY_KP_0 = 320,      // Key: Keypad 0
      KEY_KP_1 = 321,      // Key: Keypad 1
      KEY_KP_2 = 322,      // Key: Keypad 2
      KEY_KP_3 = 323,      // Key: Keypad 3
      KEY_KP_4 = 324,      // Key: Keypad 4
      KEY_KP_5 = 325,      // Key: Keypad 5
      KEY_KP_6 = 326,      // Key: Keypad 6
      KEY_KP_7 = 327,      // Key: Keypad 7
      KEY_KP_8 = 328,      // Key: Keypad 8
      KEY_KP_9 = 329,      // Key: Keypad 9
      KEY_KP_DECIMAL = 330,      // Key: Keypad .
      KEY_KP_DIVIDE = 331,      // Key: Keypad /
      KEY_KP_MULTIPLY = 332,      // Key: Keypad *
      KEY_KP_SUBTRACT = 333,      // Key: Keypad -
      KEY_KP_ADD = 334,      // Key: Keypad +
      KEY_KP_ENTER = 335,      // Key: Keypad Enter
      KEY_KP_EQUAL = 336,      // Key: Keypad =
      // Android key buttons
      KEY_BACK = 4,        // Key: Android back button
      KEY_MENU = 82,       // Key: Android menu button
      KEY_VOLUME_UP = 24,       // Key: Android volume up button
      KEY_VOLUME_DOWN = 25        // Key: Android volume down button
   };

//   using KeyCode = KeyCode;
   enum class MouseButton {
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
   static constexpr std::array<MouseButton, 7> MouseButtonsArray = {
         MouseButton::LEFT,
         MouseButton::RIGHT,
         MouseButton::MIDDLE,
         MouseButton::BACK,
         MouseButton::FORWARD,
         MouseButton::SIDE,
         MouseButton::EXTRA,
   };

   enum class MouseCursor {
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
   inline ReyEngine::WindowSpace<ReyEngine::Pos<R_FLOAT>> getMousePos(){ ReyEngine::WindowSpace<ReyEngine::Pos<float>> ws(GetMousePosition()); return ws;}
   inline ReyEngine::Vec2<R_FLOAT> getMouseDelta(){return GetMouseDelta();}
   inline ReyEngine::Vec2<R_FLOAT> getMouseWheel(){return GetMouseWheelMoveV();}

   inline void setCursor(MouseCursor crsr){ SetMouseCursor((int)crsr);}
   inline void hideCursor(){HideCursor();}

   inline std::ostream& operator<<(std::ostream& os, const KeyCode& keyCode) {
      os << static_cast<int>(keyCode);
      return os;
   }
}

namespace TextMeasure {

}

namespace DisplayInterface {
   inline void toggleFullscreen(){ToggleFullscreen();}
}
