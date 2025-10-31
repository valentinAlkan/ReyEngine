#include "Shader.h"

using namespace std;
using namespace ReyEngine;

/////////////////////////////////////////////////////////////////////////////////////////
ReyEngine::ReyShader::ReyShader(const VertShader& vertShader)
: _vs(vertShader)
{
   if (!_vs.exists() && _vs.isRegularFile()) return;
   _shader = LoadShader(_vs.canonical().c_str(), nullptr);
}

/////////////////////////////////////////////////////////////////////////////////////////
ReyEngine::ReyShader::ReyShader(const FragShader& fragShader)
: _fs(fragShader)
{
   if (!_fs.exists() && _fs.isRegularFile()) return;
   _shader = LoadShader(nullptr, _fs.canonical().c_str());
}

/////////////////////////////////////////////////////////////////////////////////////////
ReyEngine::ReyShader::ReyShader(const VertShader& vertShader, const FragShader& fragShader)
: _vs(vertShader)
, _fs(fragShader)
{
   if (!_vs.exists()) return;
   if (!_fs.exists()) return;
   _shader = LoadShader(_vs.canonical().c_str(), _fs.canonical().c_str());
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
}

/////////////////////////////////////////////////////////////////////////////////////////
ReyEngine::ReyShader::Attribute::Attribute(const ReyShader& reyShader, const std::string& name)
: ShaderData(reyShader, name)
{
   _location = GetShaderLocationAttrib(_shader, name.c_str());
}