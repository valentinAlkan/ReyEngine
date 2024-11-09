#pragma once
#include "DrawInterface.h"
#include <mutex>

namespace ReyEngine {
   //forward eclairs
   template<typename T>
   struct Size3;
   struct BoundingBox3D;
   namespace Internal{
       struct OFFSET;
       class Renderable3D;
   }
   namespace Collisions{
       struct RayHit3D;
       struct Ray3D;
   }

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

    struct Sphere{
        inline Sphere() = default;
        inline Sphere(const Pos3<double>& center, double radius): center(center), radius(radius){};
        Pos3<double> center;
        double radius = 0.0;
        bool collides(const Sphere& other);
        bool collides(const ReyEngine::BoundingBox3D& other);
        Sphere(const Vec3<double>& center, double radius): center(center), radius(radius){}
        /// Return a point along the surface of the sphere in the given direction. Distance is along the sphere,
        /// as if one were walking.
        /// \param direction
        /// \param distance
        /// \return
        Pos3<double> pointAlongSphere(Pos3<double> start, Vec3<double> direction, double distance){
           return {};
        }
        /// Returns the point that is in the given direction *though* the sphere.
        /// \param start
        /// \param direction
        /// \return
        std::optional<Pos<double>> pointThroughSphere(Pos3<double> start, Vec3<double> direction){
           return {};
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
      Sphere getSphere(const Circle& circle){
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
         return Sphere(projectedCenter, r);
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
      Collisions::Ray3D getRay(const Pos<double>& pos) const;
   };

   struct BoundingBox3D{
       bool collides(const BoundingBox3D& other);
       bool collides(const Sphere& other);
   protected:
       BoundingBox bb;
       friend class Sphere;
       friend class Internal::OFFSET;
   };

   namespace Internal {
       struct OFFSET {
           static constexpr size_t BB_OFF = offsetof(BoundingBox3D, bb);
       };
   };

   struct Model3D{
       /// Loads a model from memory. Each copy of the shared pointer points to the same memory.
       static std::optional<std::shared_ptr<Model3D>> load(const FileSystem::Path& path) noexcept;
       /// Loads the model from memory again, spitting out a distinct model, rather than referring to the same model that is loaded in memory.
       std::optional<std::shared_ptr<Model3D>> loadNewFromThis() noexcept;
       ~Model3D();
       inline bool isReady(){return IsModelReady(model);}
       inline operator bool(){return isReady();}
       Collisions::RayHit3D collides(const Collisions::Ray3D& ray);                       // Get collision info between ray and mesh

       //------------------
       class iterator {
       public:
           using iterator_category = std::forward_iterator_tag;
           using value_type = Mesh;
           using difference_type = std::ptrdiff_t;
           using pointer = Mesh*;
           using reference = Mesh&;
           iterator(pointer ptr) : current(ptr) {}
           reference operator*() const { return *current; }
           pointer operator->() { return current; }
           // Pre-increment
           iterator& operator++() {
               current++;
               return *this;
           }
           // Post-increment
           iterator operator++(int) {
               iterator tmp = *this;
               ++(*this);
               return tmp;
           }
           friend bool operator==(const iterator& a, const iterator& b) {return a.current == b.current;}
           friend bool operator!=(const iterator& a, const iterator& b) {return a.current != b.current;}
       private:
           pointer current;
       };
       // MeshRange class to be returned from getMeshes()
       class MeshRange {
       public:
           MeshRange(Mesh* start, Mesh* end) : m_begin(start), m_end(end) {}
           iterator begin() { return {m_begin}; }
           iterator end() { return {m_end}; }
       private:
           Mesh* m_begin;
           Mesh* m_end;
       };
       //------------------

       // Function to return MeshRange for iteration
       MeshRange getMeshes() {return {model.meshes, model.meshes + model.meshCount};}
   protected:
       const Model& getModel() const {return model;}
   private:
       Model3D(const FileSystem::Path& path);
       Model model;
       const FileSystem::File filePath;
       friend class ModelBody;
       friend class Internal::Renderable3D;
   };

    namespace Collisions{
        struct Ray3D{
            inline Ray3D(const Vec3<double>& source, const Vec3<double>& direction, std::optional<double> length = std::nullopt): source(source), direction(direction), length(length){};
            inline Ray3D(const Ray3D& other) = default;
            inline Ray3D(const Ray& other): source(other.position), direction(other.direction){}
            inline operator Ray() const {return {source, direction};}
            inline Ray3D& operator=(const Ray3D& other){
                source = other.source;
                direction = other.direction;
                length = other.length;
                return *this;
            }
            Vec3<double> source;
            Vec3<double> direction;
            std::optional<double> length;
            std::string toString() const {return "{" + source.toString() + ", " + direction.toString() + "}";}
            inline friend std::ostream& operator<<(std::ostream& os, const Ray3D& v) {os << v.toString(); return os;}
        };

        //All RayHits are implicitly considered hits - you should return an empty optional if there is no hit
        struct RayHit3D{
            inline RayHit3D(const Ray3D& source, const RayCollision& other): point(other.point), normal(other.normal), distance(other.distance), sourceRay(source){};
            inline RayHit3D(const Ray3D& sourceRay, const Vec3<double>& point, const Vec3<double>& normal, double distance): sourceRay(sourceRay){}
            inline RayHit3D(const RayHit3D& other) = default;
            inline RayHit3D& operator=(const RayHit3D& other){
                sourceRay = other.sourceRay;
                point = other.point;
                normal = other.point;
                distance = other.distance;
                return *this;
            }
            inline operator RayCollision(){return {true, (float)distance, point, normal};}
            Vec3<double> point;
            Vec3<double> normal;
            double distance = 0;
            const Ray3D& getSourceRay(){return sourceRay;}
            /// Get a ray that points back at the source ray
            Ray3D getReturnRay(){
                Vec3<double> reverseDirection = sourceRay.source - point;
                reverseDirection = reverseDirection.normalize();
                return Ray3D{point, reverseDirection, reverseDirection.length()};
            }
            inline friend std::ostream& operator<<(std::ostream& os, const RayHit3D& hit) {os << "[hit=" << hit.point << ",distance=" << hit.distance << ",normal=" << hit.normal << "}"; return os;}
        protected:
            Ray3D sourceRay;
        };

        bool collides(const BoundingBox3D&, const BoundingBox3D&);
        bool collides(const BoundingBox3D&, const Sphere&);
        bool collides(const Sphere&, const Sphere&);
        bool collides(const Sphere&, const Sphere&);
        std::optional<RayHit3D> collides(const Ray3D&, const Mesh&);
    }

   inline void drawSphere(const Sphere&s, int rings, int slices, const ColorRGBA& color){
      DrawSphereEx(s.center, s.radius, rings, slices, color);
   }
}
