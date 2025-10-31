#pragma once
#include "ReyEngine.h"
#include "FileSystem.h"
#include <concepts>

namespace ReyEngine {
   // Concept to check if a type is a basic shader type (not Matrix or Texture)
   template<typename T>
   concept is_supported_shader_type =
      std::is_same_v<T, float> ||
      std::is_same_v<T, Vec2<float>> ||
      std::is_same_v<T, Vec3<float>> ||
      std::is_same_v<T, Vec4<float>> ||
      std::is_same_v<T, ColorRGBA> ||
      std::is_same_v<T, int> ||
      std::is_same_v<T, Vec2<int>> ||
      std::is_same_v<T, Vec3<int>> ||
      std::is_same_v<T, Vec4<int>>;

   class ReyShader {
   public:
      // Shader uniform data type
      enum class UniformType{
         UNIFORM_FLOAT = SHADER_UNIFORM_FLOAT,       // Shader uniform type: float
         UNIFORM_VEC2 = SHADER_UNIFORM_VEC2,            // Shader uniform type: vec2 (2 float)
         UNIFORM_VEC3 = SHADER_UNIFORM_VEC3,            // Shader uniform type: vec3 (3 float)
         UNIFORM_VEC4 = SHADER_UNIFORM_VEC4,            // Shader uniform type: vec4 (4 float)
         UNIFORM_INT = SHADER_UNIFORM_INT,             // Shader uniform type: int
         UNIFORM_IVEC2 = SHADER_UNIFORM_IVEC2,           // Shader uniform type: ivec2 (2 int)
         UNIFORM_IVEC3 = SHADER_UNIFORM_IVEC3,           // Shader uniform type: ivec3 (3 int)
         UNIFORM_IVEC4 = SHADER_UNIFORM_IVEC4,           // Shader uniform type: ivec4 (4 int)
         UNIFORM_SAMPLER2D = SHADER_UNIFORM_SAMPLER2D        // Shader uniform type: sampler2d
      };

      struct FragShader : public FileSystem::File {template <typename... Args> FragShader(Args... args): FileSystem::File(args...){}};
      struct VertShader : public FileSystem::File {template <typename... Args> VertShader(Args... args): FileSystem::File(args...){}};

      struct ShaderData {
      protected:
         ShaderData(ReyShader& reyShader, const std::string& name): _shader(reyShader._shader){}
         const std::string name;
         int _location;
         Shader& _shader;
         UniformType _type;
      };
      struct Uniform : public ShaderData {
      protected:
         Uniform(ReyShader& reyShader, const std::string& name);
      };
      struct Attribute : public ShaderData {
      protected:
         Attribute(ReyShader& reyShader, const std::string& name);
      };

      template <std::derived_from<ShaderData> T, typename V>
      struct ShaderValue : public T {
         ShaderValue(ReyShader& shader, const std::string& name)
         : T(shader, name)
         {
            if constexpr (std::is_same_v<V, float>){
               T::_type = UniformType::UNIFORM_FLOAT;
            } else if constexpr (std::is_same_v<V, Vec2<float>>){
               T::_type = UniformType::UNIFORM_VEC2;
            } else if constexpr (std::is_same_v<V, Vec3<float>>){
               T::_type = UniformType::UNIFORM_VEC3;
            } else if constexpr (std::is_same_v<V, Vec4<float>>){
               T::_type = UniformType::UNIFORM_VEC4;
            } else if constexpr (std::is_same_v<V, ColorRGBA>){
               T::_type = UniformType::UNIFORM_VEC4;
            } else if constexpr (std::is_same_v<V, int>){
               T::_type = UniformType::UNIFORM_INT;
            } else if constexpr (std::is_same_v<V, Vec2<int>>){
               T::_type = UniformType::UNIFORM_IVEC2;
            } else if constexpr (std::is_same_v<V, Vec3<int>>){
               T::_type = UniformType::UNIFORM_IVEC3;
            } else if constexpr (std::is_same_v<V, Vec4<int>>){
               T::_type = UniformType::UNIFORM_IVEC4;
            } else if constexpr (std::is_same_v<V, ReyTexture>){
               T::_type = UniformType::UNIFORM_SAMPLER2D;
            } else {
               static_assert(always_false_v<V>, "Unsupported shader dataType!");
            }
         }
         ShaderValue& operator=(const V& rhs){
            if constexpr (std::is_same_v<V, Matrix>){
               SetShaderValueMatrix(T::_shader, T::_location, rhs);
            } else if constexpr (std::is_same_v<V, ReyTexture>){
               SetShaderValueTexture(T::_shader, T::_location, rhs.getTexture());
            } else if constexpr (std::is_same_v<V, ColorRGBA>){
               Vec4<float> color = {rhs.r / (float)255.0, rhs.g / (float)255.0, rhs.b / (float)255.0, rhs.a / (float)255.0};
               SetShaderValue(T::_shader, T::_location, &color, (int)T::_type);
            } else if constexpr (is_supported_shader_type<V>){
               SetShaderValue(T::_shader, T::_location, static_cast<const void*>(&rhs), (int)T::_type);
            } else {
               static_assert(always_false_v<V>, "Unsupported shader type in operator=!");
            }
            return *this;
         }
      };
      ReyShader(const VertShader&);
      ReyShader(const FragShader&);
      ReyShader(const VertShader&, const FragShader&);
      ~ReyShader();
      [[nodiscard]] bool valid(){return IsShaderReady(_shader);}
      template <typename T>
      [[nodiscard]] ShaderValue<Attribute, T> getAttribute(const std::string& name){
         return ShaderValue<Attribute, T>(*this, name);
      }
      template <typename T>
      [[nodiscard]] ShaderValue<Uniform, T> getUniform(const std::string& name){
         return ShaderValue<Uniform, T>(*this, name);
      }
      [[nodiscard]] const Shader& getShader() const {return _shader;}
   protected:
      Shader _shader = {0};
      VertShader _vs;
      FragShader _fs;
   };
}
