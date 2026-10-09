#include "Canvas3D.h"

using namespace std;
using namespace ReyEngine;

namespace {
   // Blinn-Phong lighting with directional and point lights. Assumes GL 3.3 (raylib desktop default).
   constexpr char LIGHT_VS[] = R"(
#version 330
in vec3 vertexPosition;
in vec3 vertexNormal;
in vec4 vertexColor;
uniform mat4 mvp;
uniform mat4 matModel;
uniform mat4 matNormal;
out vec3 fragPosition;
out vec3 fragNormal;
out vec4 fragColor;
void main() {
    fragPosition = vec3(matModel * vec4(vertexPosition, 1.0));
    fragNormal = normalize(vec3(matNormal * vec4(vertexNormal, 0.0)));
    fragColor = vertexColor;
    gl_Position = mvp * vec4(vertexPosition, 1.0);
}
)";

   constexpr char LIGHT_FS[] = R"(
#version 330
#define MAX_LIGHTS 8
#define LIGHT_DIRECTIONAL 0
#define LIGHT_POINT 1
in vec3 fragPosition;
in vec3 fragNormal;
in vec4 fragColor;
uniform vec4 colDiffuse;
uniform vec3 viewPos;
uniform vec3 ambient;
uniform int lightCount;
uniform int lightType[MAX_LIGHTS];
uniform vec3 lightVector[MAX_LIGHTS];
uniform vec3 lightColor[MAX_LIGHTS]; // premultiplied by intensity
uniform float lightRange[MAX_LIGHTS];
out vec4 finalColor;
void main() {
    vec3 base = colDiffuse.rgb * fragColor.rgb;
    vec3 n = normalize(fragNormal);
    vec3 v = normalize(viewPos - fragPosition);
    vec3 result = ambient * base;
    for (int i = 0; i < lightCount; i++) {
        vec3 l;
        float attenuation = 1.0;
        if (lightType[i] == LIGHT_DIRECTIONAL) {
            l = -normalize(lightVector[i]);
        } else {
            vec3 toLight = lightVector[i] - fragPosition;
            float dist = length(toLight);
            l = toLight / dist;
            if (lightRange[i] > 0.0) {
                attenuation = clamp(1.0 - dist / lightRange[i], 0.0, 1.0);
                attenuation *= attenuation;
            }
        }
        float ndl = max(dot(n, l), 0.0);
        float spec = ndl > 0.0 ? pow(max(dot(n, normalize(l + v)), 0.0), 32.0) : 0.0;
        result += (base * ndl + vec3(0.3) * spec) * lightColor[i] * attenuation;
    }
    finalColor = vec4(result, colDiffuse.a * fragColor.a);
}
)";

   Vec3<float> colorToVec3(const ColorRGBA& c, float scale = 1.0f) {
      return {c.r / 255.0f * scale, c.g / 255.0f * scale, c.b / 255.0f * scale};
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
void Canvas3D::loadGpuResources() {
   _cubeMesh = ReyMesh::makeCube();
   _sphereMesh = ReyMesh::makeSphere();
   _unlitMaterial = make_unique<ReyMaterial>();
   _litMaterial = make_unique<ReyMaterial>();

   _lightShader = ReyShader::makeShader(ShaderPrototype::fromMemory(LIGHT_VS, LIGHT_FS));
   _lightShader->setLocation(SHADER_LOC_MATRIX_MODEL, "matModel");
   _lightShader->setLocation(SHADER_LOC_MATRIX_NORMAL, "matNormal");
   auto& shader = *_lightShader;
   _uniforms.viewPos     = shader.getUniform<Vec3<float>>("viewPos");
   _uniforms.ambient     = shader.getUniform<Vec3<float>>("ambient");
   _uniforms.lightCount  = shader.getUniform<int>("lightCount");
   _uniforms.lightType   = shader.getUniform<int>("lightType[0]");
   _uniforms.lightVector = shader.getUniform<Vec3<float>>("lightVector[0]");
   _uniforms.lightColor  = shader.getUniform<Vec3<float>>("lightColor[0]");
   _uniforms.lightRange  = shader.getUniform<float>("lightRange[0]");
   _litMaterial->setShader(_lightShader);
   _gpuLoaded = true;
}

/////////////////////////////////////////////////////////////////////////////////////////
void Canvas3D::uploadLightUniforms() {
   std::array<int, MAX_LIGHTS> types{};
   std::array<Vec3<float>, MAX_LIGHTS> vectors;
   std::array<Vec3<float>, MAX_LIGHTS> colors;
   std::array<float, MAX_LIGHTS> ranges{};
   int count = 0;
   for (const auto& light : _lights) {
      if (!light.enabled) continue;
      types[count]   = light.type == Light3D::Type::DIRECTIONAL ? 0 : 1;
      vectors[count] = light.vector;
      colors[count]  = colorToVec3(light.color, light.intensity);
      ranges[count]  = light.range;
      count++;
   }
   _uniforms.viewPos.setValue(_camera3D.position);
   _uniforms.ambient.setValue(colorToVec3(_ambient));
   _uniforms.lightCount.setValue(count);
   _uniforms.lightType.setValues(std::span(types).first(count));
   _uniforms.lightVector.setValues(std::span(vectors).first(count));
   _uniforms.lightColor.setValues(std::span(colors).first(count));
   _uniforms.lightRange.setValues(std::span(ranges).first(count));
}

/////////////////////////////////////////////////////////////////////////////////////////
void Canvas3D::renderProcess(RenderContext&) {
   // The parent canvas has already bound our render target (see Canvas::doRender),
   // and will draw it onto itself once we return.
   if (!_visible) return;
   if (!_gpuLoaded) loadGpuResources();
   Internal::flushOrphaned3DResources();
   ClearBackground(Colors::transparent);

   Camera rCam{};
   rCam.position   = _camera3D.position;
   rCam.target     = _camera3D.target;
   rCam.up         = _camera3D.up;
   rCam.fovy       = _camera3D.fovy;
   rCam.projection = CAMERA_PERSPECTIVE;

   ReyMaterial* material = _unlitMaterial.get();
   if (!_lights.empty()) {
      uploadLightUniforms();
      material = _litMaterial.get();
   }

   BeginMode3D(rCam);
   for (const auto& shape : _shapes) {
      material->setColor(shape.color);
      Matrix xform = MatrixMultiply(MatrixScale(shape.size.x, shape.size.y, shape.size.z),
                                    MatrixTranslate(shape.pos.x, shape.pos.y, shape.pos.z));
      switch (shape.type) {
         case Shape::Type::CUBE:   DrawMesh(_cubeMesh->getMesh(), material->getMaterial(), xform); break;
         case Shape::Type::SPHERE: DrawMesh(_sphereMesh->getMesh(), material->getMaterial(), xform); break;
      }
   }
   EndMode3D();
}

/////////////////////////////////////////////////////////////////////////////////////////
void Canvas3D::drawCube(const Vec3<float>& pos, const Vec3<float>& size, const ColorRGBA& color){
   _shapes.push_back({Shape::Type::CUBE, pos, size, color});
}

/////////////////////////////////////////////////////////////////////////////////////////
void Canvas3D::drawSphere(const Vec3<float>& pos, float radius, const ColorRGBA& color){
   _shapes.push_back({Shape::Type::SPHERE, pos, {radius, radius, radius}, color});
}

/////////////////////////////////////////////////////////////////////////////////////////
size_t Canvas3D::addDirectionalLight(const Vec3<float>& direction, const ColorRGBA& color, float intensity) {
   if (_lights.size() >= MAX_LIGHTS) throw ReyError("Canvas3D: too many lights");
   _lights.push_back({Light3D::Type::DIRECTIONAL, direction, color, intensity});
   return _lights.size() - 1;
}

/////////////////////////////////////////////////////////////////////////////////////////
size_t Canvas3D::addPointLight(const Vec3<float>& position, const ColorRGBA& color, float intensity, float range) {
   if (_lights.size() >= MAX_LIGHTS) throw ReyError("Canvas3D: too many lights");
   _lights.push_back({Light3D::Type::POINT, position, color, intensity, range});
   return _lights.size() - 1;
}
