#include "Shader.h"

using namespace std;
using namespace ReyEngine;

std::shared_ptr<ReyShader> ReyShader::_global_default_fragment_shader;
std::shared_ptr<ReyShader> ReyShader::getDefaultFragmentShader() {
   if (!_global_default_fragment_shader){
      static constexpr char SHADER_TEXT[] =R"(
         #version 330 core
         uniform sampler2D inputTexture;
         in vec2 v_uv;
         out vec4 fragColor;
         void main(){
         fragColor = texture(inputTexture, v_uv);
         };)";
      _global_default_fragment_shader = ReyShader::makeFragment(FragmentOnlyShaderPrototype::fromMemory(SHADER_TEXT));
   }
   return _global_default_fragment_shader;
}

/////////////////////////////////////////////////////////////////////////////////////////
ReyEngine::ReyShader::~ReyShader(){
   if (valid()){
      UnloadShader(_shader);
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
ReyEngine::ReyShader::Uniform::Uniform(const ReyShader& reyShader, const std::string& name)
: ShaderData(reyShader, name)
{
   _location = GetShaderLocation(_shader, name.c_str());
   if (_location < 0) Logger::error() << "Unable to find shader uniform location: " << name << endl;
}

/////////////////////////////////////////////////////////////////////////////////////////
ReyEngine::ReyShader::Attribute::Attribute(const ReyShader& reyShader, const std::string& name)
: ShaderData(reyShader, name)
{
   _location = GetShaderLocationAttrib(_shader, name.c_str());
   if (_location < 0) Logger::error() << "Unable to find shader uniform location: " << name << endl;
}