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

   namespace Internal {
      enum class ShaderType {VERTEX, FRAGMENT, BOTH};
      template <ShaderType type, typename... Args>
      static inline std::optional<Shader> loadShaderfromFile(Args... args) {
         Shader shader;
         if constexpr (type == ShaderType::BOTH) {
            auto arg_tuple = std::make_tuple(args...);
            static_assert(sizeof...(Args) >= 2, "ShaderType::BOTH requires at least two arguments.");
            FileSystem::File fvs(std::get<0>(arg_tuple));
            FileSystem::File ffs(std::get<1>(arg_tuple));
            if (!fvs.exists() || !ffs.exists() || !fvs.isRegularFile() || !ffs.isRegularFile()) return {};
            shader = LoadShader(fvs.canonical().c_str(), ffs.canonical().c_str());
         } else {
            static_assert(sizeof...(Args) == 1, "ShaderType::VERTEX/FRAGMENT requires exactly one argument.");
            FileSystem::File f(args...);
            if (!f.exists() || !f.isRegularFile()) return {};
            if constexpr (type == ShaderType::FRAGMENT) {
               shader = LoadShader(nullptr, f.canonical().c_str());
            } else if constexpr (type == ShaderType::VERTEX) {
               shader = LoadShader(f.canonical().c_str(), nullptr);
            }
         }
         if (IsShaderReady(shader)) return shader;
         return {};
      }

      template <ShaderType type, typename... Args>
      static inline std::optional<Shader> loadShaderfromString(Args&&... args) {
         Shader shader;
         if constexpr (type == ShaderType::BOTH) {
            auto arg_tuple = std::make_tuple(std::forward<Args>(args)...);
            static_assert(sizeof...(Args) == 2, "ShaderType::BOTH requires exactly two string arguments (VS source, FS source).");
            const char* vs_source = std::get<0>(arg_tuple).c_str();
            const char* fs_source = std::get<1>(arg_tuple).c_str();
            shader = LoadShaderFromMemory(vs_source, fs_source);
         } else {
            static_assert(sizeof...(Args) == 1, "ShaderType::VERTEX or FRAGMENT requires exactly one string argument (source code).");
            auto arg_tuple = std::make_tuple(std::forward<Args>(args)...);
            auto& source = std::get<0>(arg_tuple);
            Logger::info() << "Loading Shader:\n" << source << std::endl;
            if constexpr (type == ShaderType::FRAGMENT) {
               shader = LoadShaderFromMemory(nullptr, source.c_str());
            } else if constexpr (type == ShaderType::VERTEX) {
               shader = LoadShaderFromMemory(source.c_str(), nullptr);
            }
         }
         if (IsShaderReady(shader)) return shader;
         return {};
      }
      struct ShaderPrototype{Shader shader;};
   }

   #define _throw throw std::runtime_error("Invalid shader!");
   struct FragmentOnlyShaderPrototype : Internal::ShaderPrototype {
      static FragmentOnlyShaderPrototype fromFile(const ReyEngine::FileSystem::File& file){
         if (auto proto = Internal::loadShaderfromFile<Internal::ShaderType::FRAGMENT>(file)){
            FragmentOnlyShaderPrototype s;
            s.shader = proto.value();
            return s;
         }
         _throw;
      }
      static FragmentOnlyShaderPrototype fromMemory(const std::string& data){
         if (auto proto = Internal::loadShaderfromString<Internal::ShaderType::FRAGMENT>(data)){
            FragmentOnlyShaderPrototype s;
            s.shader = proto.value();
            return s;
         }
         _throw;
      }
   };

   struct VertexOnlyShaderPrototype : public ReyEngine::Internal::ShaderPrototype {
      static VertexOnlyShaderPrototype fromFile(const ReyEngine::FileSystem::File& file){
         if (auto proto = Internal::loadShaderfromFile<Internal::ShaderType::FRAGMENT>(file)){
            VertexOnlyShaderPrototype s;
            s.shader = proto.value();
            return s;
         }
         _throw;
      }
      static VertexOnlyShaderPrototype fromMemory(const std::string& data){
         if (auto proto = Internal::loadShaderfromString<Internal::ShaderType::FRAGMENT>(data)){
            VertexOnlyShaderPrototype s;
            s.shader = proto.value();
            return s;
         }
         _throw;
      }
   };

   struct ShaderPrototype : public ReyEngine::Internal::ShaderPrototype {
      static ShaderPrototype fromFile(const ReyEngine::FileSystem::File& vs, const ReyEngine::FileSystem::File& fs){
         if (auto proto = Internal::loadShaderfromFile<Internal::ShaderType::BOTH>(vs, fs)){
            ShaderPrototype s;
            s.shader = proto.value();
            return s;
         }
         _throw;
      }
      static ShaderPrototype fromMemory(const std::string& vsData, const std::string& fsData){
         if (auto proto = Internal::loadShaderfromString<Internal::ShaderType::BOTH>(vsData, fsData)){
            ShaderPrototype s;
            s.shader = proto.value();
            return s;
         }
         _throw;
      }
   };
   #undef _throw

   class ReyShader {
   public:
      enum class SourceType {FROM_FILE, FROM_STRING};
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

      struct ShaderData {
         int getLocation(){return _location;}
         int getType(){return (int)_type;}
      protected:
         ShaderData() = default;
         ShaderData(const ReyShader& reyShader, const std::string& name)
         : _shader(reyShader._shader)
         , _name(name)
         {}
         std::string _name;
         int _location;
         Shader _shader = {0}; //doesnt' do memory cleanup so stack-o-lee it is
         UniformType _type;
      };
      struct Uniform : public ShaderData {
      protected:
         Uniform() = default;
         Uniform(const ReyShader& reyShader, const std::string& name);
      };
      struct Attribute : public ShaderData {
      protected:
         Attribute() = default;
         Attribute(const ReyShader& reyShader, const std::string& name);
      };

      template <std::derived_from<ShaderData> T, typename V>
      struct ShaderValue : public T {
         ShaderValue() = default; //null shader
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
         void setValue(const V& rhs){
            if constexpr (std::is_same_v<V, Matrix>){
               SetShaderValueMatrix(T::_shader, T::_location, rhs);
            } else if constexpr (std::is_same_v<V, ReyTexture>){
               SetShaderValueTexture(T::_shader, T::_location, rhs.getTexture());
            } else if constexpr (std::is_same_v<V, ColorRGBA>){
               Vec4<float> color = {rhs.r / (float)255.0, rhs.g / (float)255.0, rhs.b / (float)255.0, rhs.a / (float)255.0};
               SetShaderValue(T::_shader, T::_location, static_cast<void*>(&color), (int)T::_type);
            } else if constexpr (is_supported_shader_type<V>){
               SetShaderValue(T::_shader, T::_location, static_cast<const void*>(&rhs), (int)T::_type);
            } else {
               static_assert(always_false_v<V>, "Unsupported shader type in operator=!");
            }
         }
      };
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
      static std::shared_ptr<ReyShader> makeFragment(const FragmentOnlyShaderPrototype& p){return std::shared_ptr<ReyShader>(new ReyShader(p));}
      static std::shared_ptr<ReyShader> makeVertex(const VertexOnlyShaderPrototype& p){return std::shared_ptr<ReyShader>(new ReyShader(p));}
      static std::shared_ptr<ReyShader> makeShader(const ShaderPrototype& p){return std::shared_ptr<ReyShader>(new ReyShader(p));}
   protected:
      ReyShader(const FragmentOnlyShaderPrototype& s): _shader(s.shader){}
      ReyShader(const VertexOnlyShaderPrototype& s): _shader(s.shader){}
      ReyShader(const ShaderPrototype& s): _shader(s.shader){}
      Shader _shader = {0};
   };
}
