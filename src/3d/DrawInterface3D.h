#pragma once
#include "raylib.h"
#include "DrawInterface.h"

namespace ReyEngine {
   //forward eclar
   template<typename T>
   struct Size3;

   template <typename T>
   struct Pos3 : public Vec3<T>{
      inline Pos3(): Vec3<T>(){}
      inline Pos3(const T& x, const T& y) : Vec3<T>(x, y){}
      inline Pos3(const Vector2& v)     : Vec3<T>(v){}
      inline Pos3(const Vec3<int>& v)   : Vec3<T>(v){}
      inline Pos3(const Vec3<double>& v): Vec3<T>(v){}
      inline Pos3(const Vec3<float>& v) : Vec3<T>(v){}
      inline void operator=(Size3<T>&) = delete;
      inline Pos3 operator+(const Pos3& rhs) const {auto val = *this; val.x += rhs.x; val.y += rhs.y; return val;}
      inline Pos3 operator-(const Pos3& rhs) const {auto val = *this; val.x -= rhs.x; val.y -= rhs.y; return val;}
      inline Pos3& operator+=(const Pos3& rhs){this->x += rhs.x; this->y += rhs.y; return *this;}
      inline Pos3& operator-=(const Pos3& rhs){this->x -= rhs.x; this->y -= rhs.y; return *this;}
      inline bool operator!=(const Pos3& rhs){return this->x != rhs.x || this->y != rhs.y;}
      inline operator std::string() const {return Vec3<T>::toString();}
      inline void operator=(const Size<T>&) = delete;
      inline Pos3& operator=(const Pos3<T>& other){Pos3::x = other.x; Pos3::y = other.y; return *this;}
      inline Pos3& operator=(const Vec3<T>& other){Pos3::x = other.x; Pos3::y = other.y; return *this;}
      inline std::string toString() const {return Vec3<T>::toString();}
   };

   template <typename T>
   struct Size3 : public Vec3<T>{
      inline Size3(): Vec3<T>(){}
      inline Size3(const T& x, const T& y) : Vec3<T>(x, y){}
      inline Size3(const Vector2& v)     : Vec3<T>(v){}
      inline Size3(const Vec3<int>& v)   : Vec3<T>(v){}
      inline Size3(const Vec3<double>& v): Vec3<T>(v){}
      inline Size3(const Vec3<float>& v) : Vec3<T>(v){}
      inline void operator=(Size<T>&) = delete;
      inline Size3 operator+(const Size3& rhs) const {auto val = *this; val.x += rhs.x; val.y += rhs.y; return val;}
      inline Size3 operator-(const Size3& rhs) const {auto val = *this; val.x -= rhs.x; val.y -= rhs.y; return val;}
      inline Size3& operator+=(const Size3& rhs){this->x += rhs.x; this->y += rhs.y; return *this;}
      inline Size3& operator-=(const Size3& rhs){this->x -= rhs.x; this->y -= rhs.y; return *this;}
      inline bool operator!=(const Size3& rhs){return this->x != rhs.x || this->y != rhs.y;}
      inline operator std::string() const {return Vec3<T>::toString();}
      inline void operator=(const Pos3<T>&) = delete;
      inline Size3& operator=(const Size3<T>& other){Size3::x = other.x; Size3::y = other.y; return *this;}
      inline Size3& operator=(const Vec3<T>& other){Size3::x = other.x; Size3::y = other.y; return *this;}
      inline std::string toString() const {return Vec3<T>::toString();}
   };

   struct CameraTransform3D{
      CameraTransform3D();
      Camera3D camera;
      void setFOV(float newFOV);
      void push() const;
      void pop();
   };
}
