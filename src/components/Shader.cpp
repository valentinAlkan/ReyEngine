#include "Shader.h"

using namespace std;
using namespace ReyEngine;

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