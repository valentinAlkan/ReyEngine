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

   Vector3 colorToVec3(const ColorRGBA& c, float scale = 1.0f) {
      return {c.r / 255.0f * scale, c.g / 255.0f * scale, c.b / 255.0f * scale};
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
Canvas3D::~Canvas3D() {
   if (!_gpuLoaded) return;
   UnloadMesh(_cubeMesh);
   UnloadMesh(_sphereMesh);
   UnloadMaterial(_litMaterial); // also unloads the lighting shader
   UnloadMaterial(_unlitMaterial);
}

/////////////////////////////////////////////////////////////////////////////////////////
void Canvas3D::loadGpuResources() {
   _cubeMesh = GenMeshCube(1, 1, 1);
   _sphereMesh = GenMeshSphere(1, 32, 32);
   _unlitMaterial = LoadMaterialDefault();
   _litMaterial = LoadMaterialDefault();

   Shader shader = LoadShaderFromMemory(LIGHT_VS, LIGHT_FS);
   if (!IsShaderValid(shader)) throw ReyError("Canvas3D: failed to compile lighting shader");
   shader.locs[SHADER_LOC_MATRIX_MODEL]  = GetShaderLocation(shader, "matModel");
   shader.locs[SHADER_LOC_MATRIX_NORMAL] = GetShaderLocation(shader, "matNormal");
   _locs.viewPos     = GetShaderLocation(shader, "viewPos");
   _locs.ambient     = GetShaderLocation(shader, "ambient");
   _locs.lightCount  = GetShaderLocation(shader, "lightCount");
   _locs.lightType   = GetShaderLocation(shader, "lightType[0]");
   _locs.lightVector = GetShaderLocation(shader, "lightVector[0]");
   _locs.lightColor  = GetShaderLocation(shader, "lightColor[0]");
   _locs.lightRange  = GetShaderLocation(shader, "lightRange[0]");
   _litMaterial.shader = shader;
   _gpuLoaded = true;
}

/////////////////////////////////////////////////////////////////////////////////////////
void Canvas3D::uploadLightUniforms() {
   int types[MAX_LIGHTS];
   Vector3 vectors[MAX_LIGHTS];
   Vector3 colors[MAX_LIGHTS];
   float ranges[MAX_LIGHTS];
   int count = 0;
   for (const auto& light : _lights) {
      if (!light.enabled) continue;
      types[count]   = light.type == Light3D::Type::DIRECTIONAL ? 0 : 1;
      vectors[count] = (Vector3)light.vector;
      colors[count]  = colorToVec3(light.color, light.intensity);
      ranges[count]  = light.range;
      count++;
   }
   auto& shader = _litMaterial.shader;
   Vector3 viewPos = (Vector3)_camera3D.position;
   Vector3 ambient = colorToVec3(_ambient);
   SetShaderValue(shader, _locs.viewPos, &viewPos, SHADER_UNIFORM_VEC3);
   SetShaderValue(shader, _locs.ambient, &ambient, SHADER_UNIFORM_VEC3);
   SetShaderValue(shader, _locs.lightCount, &count, SHADER_UNIFORM_INT);
   if (count == 0) return;
   SetShaderValueV(shader, _locs.lightType, types, SHADER_UNIFORM_INT, count);
   SetShaderValueV(shader, _locs.lightVector, vectors, SHADER_UNIFORM_VEC3, count);
   SetShaderValueV(shader, _locs.lightColor, colors, SHADER_UNIFORM_VEC3, count);
   SetShaderValueV(shader, _locs.lightRange, ranges, SHADER_UNIFORM_FLOAT, count);
}

/////////////////////////////////////////////////////////////////////////////////////////
void Canvas3D::renderProcess(RenderContext&) {
   // The parent canvas has already bound our render target (see Canvas::doRender),
   // and will draw it onto itself once we return.
   if (!_visible) return;
   if (!_gpuLoaded) loadGpuResources();
   ClearBackground(Colors::transparent);

   Camera rCam{};
   rCam.position   = _camera3D.position;
   rCam.target     = _camera3D.target;
   rCam.up         = _camera3D.up;
   rCam.fovy       = _camera3D.fovy;
   rCam.projection = CAMERA_PERSPECTIVE;

   Material* material = &_unlitMaterial;
   if (!_lights.empty()) {
      uploadLightUniforms();
      material = &_litMaterial;
   }

   BeginMode3D(rCam);
   for (const auto& shape : _shapes) {
      material->maps[MATERIAL_MAP_DIFFUSE].color = (Color)shape.color;
      Matrix xform = MatrixMultiply(MatrixScale(shape.size.x, shape.size.y, shape.size.z),
                                    MatrixTranslate(shape.pos.x, shape.pos.y, shape.pos.z));
      switch (shape.type) {
         case Shape::Type::CUBE:   DrawMesh(_cubeMesh, *material, xform); break;
         case Shape::Type::SPHERE: DrawMesh(_sphereMesh, *material, xform); break;
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
