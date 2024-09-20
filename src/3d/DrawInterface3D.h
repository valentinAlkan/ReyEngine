#pragma once
#include "DrawInterface.h"

namespace ReyEngine {
   //forward eclar
   template<typename T>
   struct Size3;

   template <typename T>
   struct Pos3 : public Vec3<T>{
      inline Pos3(): Vec3<T>(){}
      inline Pos3(const T& x, const T& y, const T& z) : Vec3<T>(x, y, z){}
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

   template <typename T>
   struct Line3D{
      Pos3<T> x;
      Pos3<T> y;
      Pos3<T> z;
   };

   template <typename T>
   struct Sphere {
      Sphere(const Vec3<T>& center, double radius): center(center), radius(radius){}
      Vec3<T> center;
      T radius;
      /// Return a point along the surface of the sphere in the given direction. Distance is along the sphere,
      /// as if one were walking.
      /// \param direction
      /// \param distance
      /// \return
      Pos3<T> pointAlongSphere(Pos3<T> start, Vec3<T> direction, T distance){

      }
      /// Returns the point that is in the given direction *though* the sphere.
      /// \param start
      /// \param direction
      /// \return
      std::optional<Pos<T>> pointThroughSphere(Pos3<T> start, Vec3<T> direction){

      }
   };

   /// A plane in 3D space. The normal direction is defined in the same manner that opengl
   /// determines the normal for a triangle - ie clockwise from an observers perspective of the 'front'
   /// (the front being the direction in which the normal points)
   /// \tparam T
   template <typename T>
   struct Plane {
      Plane(const Pos3<T>& a, const Pos3<T>& b, const Pos3<T>& c)
      {
         Vec3<T> v1 = b - a;
         Vec3<T> v2 = c - a;
         Vec3<T> crossProduct = v1.cross(v2);
         _normal = crossProduct.normalize();
      }
      Pos3<T> a;
      Pos3<T> b;
      Pos3<T> c;
      const Vec3<T>& getNormal(){return _normal;}

      /// Create a sphere by projecting the circle onto the plane such that the plane represents the equator of the sphere.
      /// \param circle: a circle
      /// \return A sphere
      Sphere<T> getSphere(const Circle& circle){
         Vec3<T> planePoint(a.x, a.y, a.z);
         Vec3<T> circleCenter2D(circle.center.x, circle.center.y, 0);

         // dot product between the normal and the vector from planePoint to the circle center
         T dot = _normal.x * (circleCenter2D.x - planePoint.x) +
                 _normal.y * (circleCenter2D.y - planePoint.y);

         // Project the circle center onto the plane's z-coordinate
         Vec3<T> projectedCenter(
               circleCenter2D.x - dot * _normal.x,
               circleCenter2D.y - dot * _normal.y,
               planePoint.z
         );

         T r = static_cast<T>(circle.radius);
         return Sphere<T>(projectedCenter, r);
      }
   private:
      Vec3<T> _normal;
   };

   struct CameraTransform3D{
      CameraTransform3D();
      Camera3D camera;
      void setFOV(float newFOV);
      void push() const;
      void pop();
   };

   void drawSphere(const Sphere<double>&s, int rings, int slices, const ColorRGBA& color){
      DrawSphereEx(s.center, s.radius, rings, slices, color);
   }
}
