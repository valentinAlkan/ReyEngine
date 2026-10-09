#pragma once
#include "ReyEngine.h"

// 3D counterparts of the 2D geometry types: Pos3 <-> Pos, Size3 <-> Size, Box <-> Rect
namespace ReyEngine {
   template <typename T> struct Box;

   template <typename T=R_FLOAT>
   struct Pos3 : public Vec3<T> {
      constexpr inline Pos3(): Vec3<T>(){}
      constexpr inline Pos3(const T& x, const T& y, const T& z) : Vec3<T>(x, y, z){}
      constexpr inline Pos3(const Vector3& v) : Vec3<T>(v){}
      template <typename R>
      constexpr explicit inline Pos3(const Vec3<R>& v) : Vec3<T>(v){}
      constexpr inline Pos3 operator+(const Pos3& rhs) const {return Pos3(Vec3<T>::operator+(rhs));}
      constexpr inline Pos3 operator-(const Pos3& rhs) const {return Pos3(Vec3<T>::operator-(rhs));}
      constexpr inline Pos3 operator-() const {return Pos3(Vec3<T>::operator-());}
      constexpr inline Pos3 operator*(float amt) const {return Pos3(Vec3<T>::operator*(amt));}
      constexpr inline Pos3 operator/(float amt) const {return Pos3(Vec3<T>::operator/(amt));}
      [[nodiscard]] inline Pos3 transform(const Matrix& m) const {return Pos3(Vec3<T>::transform(m));}
      [[nodiscard]] inline Box<T> toBox(const Vec3<T>& size) const;
      inline static std::optional<Pos3<T>> fromString(const std::string& s){
         if (auto optVec3 = Vec3<T>::fromString(s)) return Pos3<T>(optVec3.value());
         return {};
      }
      inline operator std::string() const {return Vec3<T>::toString();}
   };

   template <typename T=R_FLOAT>
   struct Size3 : public Vec3<T> {
      constexpr inline Size3(): Vec3<T>(){}
      constexpr inline Size3(const T& x, const T& y, const T& z) : Vec3<T>(x, y, z){}
      constexpr inline explicit Size3(const T& edge): Size3(edge, edge, edge){}
      constexpr inline Size3(const Vector3& v) : Vec3<T>(v){}
      template <typename R>
      constexpr inline Size3(const Vec3<R>& v) : Vec3<T>(v){}
      constexpr inline Size3 operator+(const Size3& rhs) const {return Size3(Vec3<T>::operator+(rhs));}
      constexpr inline Size3 operator-(const Size3& rhs) const {return Size3(Vec3<T>::operator-(rhs));}
      constexpr inline Size3 operator*(float amt) const {return Size3(Vec3<T>::operator*(amt));}
      constexpr inline Size3 operator/(float amt) const {return Size3(Vec3<T>::operator/(amt));}
      [[nodiscard]] constexpr inline Pos3<T> center() const {return {this->x/2, this->y/2, this->z/2};}
      [[nodiscard]] constexpr inline Box<T> toBox() const;
      inline explicit operator std::string() const {return Vec3<T>::toString();}
   };

   // Axis aligned box. Like Rect, it is anchored at its minimum corner (x, y, z) and extends by (width, height, depth).
   template <typename T=R_FLOAT>
   struct Box {
      constexpr inline Box(): x(0), y(0), z(0), width(0), height(0), depth(0){}
      constexpr inline Box(T x, T y, T z, T width, T height, T depth): x(x), y(y), z(z), width(width), height(height), depth(depth){}
      constexpr inline Box(const Pos3<T>& pos, const Size3<T>& size): Box(pos.x, pos.y, pos.z, size.x, size.y, size.z){}
      constexpr inline explicit Box(const BoundingBox& b): Box(Pos3<T>(b.min), Size3<T>(Vec3<T>(b.max) - Vec3<T>(b.min))){}
      template <typename R>
      constexpr inline Box(const Box<R>& b): Box((T)b.x, (T)b.y, (T)b.z, (T)b.width, (T)b.height, (T)b.depth){}
      //construct from a center point rather than the minimum corner
      constexpr inline static Box fromCenter(const Pos3<T>& center, const Size3<T>& size){return {Pos3<T>(center - size.center()), size};}
      constexpr inline operator BoundingBox() const {return {min(), max()};}
      constexpr inline bool operator==(const Box& rhs) const {return pos() == rhs.pos() && size() == rhs.size();}
      constexpr inline bool operator!=(const Box& rhs) const {return !(*this == rhs);}
      constexpr inline Box operator+(const Pos3<T>& rhs) const {return {pos() + rhs, size()};}
      constexpr inline Box operator-(const Pos3<T>& rhs) const {return {pos() - rhs, size()};}
      [[nodiscard]] constexpr inline Pos3<T> pos() const {return {x, y, z};}
      [[nodiscard]] constexpr inline Size3<T> size() const {return {width, height, depth};}
      [[nodiscard]] constexpr inline Pos3<T> min() const {return pos();}
      [[nodiscard]] constexpr inline Pos3<T> max() const {return {x + width, y + height, z + depth};}
      [[nodiscard]] constexpr inline Pos3<T> center() const {return pos() + size().center();}
      [[nodiscard]] constexpr inline bool contains(const Vec3<T>& p) const {
         return p.x >= x && p.x <= x + width && p.y >= y && p.y <= y + height && p.z >= z && p.z <= z + depth;
      }
      [[nodiscard]] constexpr inline bool intersects(const Box& other) const {
         return x <= other.x + other.width && other.x <= x + width
             && y <= other.y + other.height && other.y <= y + height
             && z <= other.z + other.depth && other.z <= z + depth;
      }
      //smallest box that contains both boxes
      [[nodiscard]] constexpr inline Box merge(const Box& other) const {
         auto lo = Pos3<T>(min().min(other.min()));
         auto hi = Pos3<T>(max().max(other.max()));
         return {lo, Size3<T>(hi - lo)};
      }
      //the axis aligned box enclosing this box after it has been transformed
      [[nodiscard]] inline Box transform(const Matrix& m) const {
         auto lo = min(), hi = max();
         Vec3<T> newLo, newHi;
         for (int i = 0; i < 8; i++) {
            Vec3<T> corner(i & 1 ? hi.x : lo.x, i & 2 ? hi.y : lo.y, i & 4 ? hi.z : lo.z);
            corner = corner.transform(m);
            newLo = i ? newLo.min(corner) : corner;
            newHi = i ? newHi.max(corner) : corner;
         }
         return {Pos3<T>(newLo), Size3<T>(newHi - newLo)};
      }
      inline std::string toString() const {return "{" + pos().toString() + ", " + size().toString() + "}";}
      friend std::ostream& operator<<(std::ostream& os, const Box& b) {os << b.toString(); return os;}
      T x;
      T y;
      T z;
      T width;
      T height;
      T depth;
   };

   /// A half-line starting at origin and heading along direction. The 3D counterpart of Line, used for picking.
   struct Ray3D {
      constexpr inline Ray3D() = default;
      constexpr inline Ray3D(const Pos3<R_FLOAT>& origin, const Vec3<R_FLOAT>& direction): origin(origin), direction(direction){}
      constexpr inline Ray3D(const Ray& r): origin(r.position), direction(r.direction){}
      constexpr inline operator Ray() const {return {origin, direction};}
      [[nodiscard]] constexpr inline Pos3<R_FLOAT> at(R_FLOAT distance) const {return Pos3<R_FLOAT>(origin + direction * distance);}
      /// the same ray, expressed in another space
      [[nodiscard]] inline Ray3D transform(const Matrix& m) const {
         auto newOrigin = origin.transform(m);
         auto newDirection = Vec3<R_FLOAT>(Pos3<R_FLOAT>(origin + direction).transform(m) - newOrigin);
         return {newOrigin, newDirection.normalize().value_or(direction)};
      }
      /// distance along the ray to where it enters the box, if it hits it
      [[nodiscard]] inline std::optional<R_FLOAT> intersects(const Box<R_FLOAT>& box) const {
         auto hit = GetRayCollisionBox(*this, box);
         if (hit.hit) return hit.distance;
         return {};
      }
      friend std::ostream& operator<<(std::ostream& os, const Ray3D& r) {os << "{" << r.origin << " -> " << r.direction << "}"; return os;}
      Pos3<R_FLOAT> origin;
      Vec3<R_FLOAT> direction;
   };

   template <typename T>
   inline Box<T> Pos3<T>::toBox(const Vec3<T>& size) const {return {*this, Size3<T>(size)};}
   template <typename T>
   constexpr inline Box<T> Size3<T>::toBox() const {return {Pos3<T>(), *this};}
}
