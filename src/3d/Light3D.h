#pragma once
#include "Node3D.h"

namespace ReyEngine {
   /// Base class for lights. Lights are nodes like anything else in the scene: they are placed and aimed with their
   /// transform, inherit their parent's transform, and are switched off by hiding them (setVisible(false)).
   /// A Canvas3D with no visible lights draws everything unlit.
   class Light3D : public Node3D {
   public:
      enum class Type {DIRECTIONAL, POINT};
      REYENGINE_OBJECT(Light3D)
      [[nodiscard]] Type getType() const {return _type;}
      void setColor(const ColorRGBA& color){_color = color;}
      [[nodiscard]] const ColorRGBA& getColor() const {return _color;}
      void setIntensity(float intensity){_intensity = intensity;}
      [[nodiscard]] float getIntensity() const {return _intensity;}
   protected:
      Light3D(Type type, const ColorRGBA& color, float intensity)
      : _type(type)
      , _color(color)
      , _intensity(intensity)
      {}
      Type _type;
      ColorRGBA _color;
      float _intensity;
   };

   /// Light arriving from one direction everywhere in the scene (sunlight). Its position is irrelevant; it shines
   /// along its local -Z axis, so rotating the node (or its parent) re-aims it.
   class DirectionalLight3D : public Light3D {
   public:
      REYENGINE_OBJECT(DirectionalLight3D)
      static constexpr Vec3<float> FORWARD = {0, 0, -1};
      explicit DirectionalLight3D(const Vec3<float>& direction = {0, -1, 0}, const ColorRGBA& color = Colors::white, float intensity = 1.0f)
      : Light3D(Type::DIRECTIONAL, color, intensity)
      {setDirection(direction);}
      /// aim the light along the given direction (in its parent's space)
      void setDirection(const Vec3<float>& direction){
         if (auto normalized = direction.normalize()) setRotation(QuaternionFromVector3ToVector3(FORWARD, normalized.value()));
      }
      /// the direction the light travels, in the owning canvas' (world) space
      [[nodiscard]] Vec3<float> getGlobalDirection() const {
         auto xform = getGlobalTransform();
         auto dir = xform.transform(FORWARD) - xform.transform({0, 0, 0});
         return dir.normalize().value_or(FORWARD);
      }
   };

   /// Light radiating from its position in every direction (a bulb).
   class PointLight3D : public Light3D {
   public:
      REYENGINE_OBJECT(PointLight3D)
      explicit PointLight3D(const ColorRGBA& color = Colors::white, float intensity = 1.0f, float range = 0.0f)
      : Light3D(Type::POINT, color, intensity)
      , _range(range)
      {}
      /// the light fades to nothing at this distance. 0 = no falloff
      void setRange(float range){_range = range;}
      [[nodiscard]] float getRange() const {return _range;}
   private:
      float _range;
   };
}
