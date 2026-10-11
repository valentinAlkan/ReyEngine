#include "Canvas3D.h"
#include "rlgl.h"

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
   _gpuLoaded = true;
}

/////////////////////////////////////////////////////////////////////////////////////////
void Canvas3D::uploadLightUniforms(const std::vector<const Light3D*>& lights) {
   std::array<int, MAX_LIGHTS> types{};
   std::array<Vec3<float>, MAX_LIGHTS> vectors;
   std::array<Vec3<float>, MAX_LIGHTS> colors;
   std::array<float, MAX_LIGHTS> ranges{};
   int count = 0;
   for (const auto& light : lights) {
      if (count == MAX_LIGHTS) {
         Logger::warn() << "Canvas3D " << getName() << ": only the first " << MAX_LIGHTS << " lights are used" << endl;
         break;
      }
      if (light->getType() == Light3D::Type::DIRECTIONAL) {
         types[count]   = 0;
         vectors[count] = static_cast<const DirectionalLight3D*>(light)->getGlobalDirection();
      } else {
         types[count]   = 1;
         vectors[count] = light->getGlobalPosition();
         ranges[count]  = static_cast<const PointLight3D*>(light)->getRange();
      }
      colors[count] = colorToVec3(light->getColor(), light->getIntensity());
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
void Canvas3D::renderBackgroundLayer(RenderContext& renderContext) {
   if (!_gpuLoaded) loadGpuResources();
   Internal::flushOrphaned3DResources();

   //materials without a shader of their own get lit when there are lights, and are drawn flat otherwise
   std::vector<const Light3D*> lights;
   for (auto& child : getChildren()) {
      if (auto drawable = child->as<Internal::Drawable3D>()) collectLights(drawable.value(), lights);
   }
   //panels have to be drawn into their textures before the 3D pass begins, since that can't be interrupted
   for (auto& child : getChildren()) {
      if (auto drawable = child->as<Internal::Drawable3D>()) renderPanels(renderContext, drawable.value());
   }

   std::shared_ptr<ReyShader> defaultShader;
   if (!lights.empty()) {
      uploadLightUniforms(lights);
      defaultShader = _lightShader;
   }

   {
      RenderContext3D context3D(defaultShader);
      auto cameraMode = renderContext.cameraContext(_camera3D);
      for (auto& child : getChildren()) {
         if (auto drawable = child->as<Internal::Drawable3D>()) {
            doRender3D(context3D, drawable.value());
         }
      }
   }

   //then any 2D widgets in the background layer
   Canvas::renderBackgroundLayer(renderContext);
}

/////////////////////////////////////////////////////////////////////////////////////////
void Canvas3D::doRender3D(RenderContext3D& context, Internal::Drawable3D* drawable) {
   if (!drawable->_visible) return;
   context.push(drawable->getLocalTransform());
   drawable->render3DBegin(context);
   drawable->render3D(context);
   drawable->render3DEnd(context);
   for (auto& child : drawable->getChildren()) {
      if (auto childDrawable = child->as<Internal::Drawable3D>()) {
         doRender3D(context, childDrawable.value());
      } else if (auto isPanel = child->as<Canvas>()) {
         drawPanel(context, isPanel.value());
      }
   }
   context.pop();
}

/////////////////////////////////////////////////////////////////////////////////////////
void Canvas3D::renderPanels(RenderContext& renderContext, Internal::Drawable3D* drawable) {
   //hidden nodes hide their descendants, panels included
   if (!drawable->_visible) return;
   for (auto& child : drawable->getChildren()) {
      if (auto childDrawable = child->as<Internal::Drawable3D>()) {
         renderPanels(renderContext, childDrawable.value());
      } else if (auto isPanel = child->as<Canvas>()) {
         auto panel = isPanel.value();
         if (panel->getVisible() && renderTargetOf(panel).ready()) renderOffscreen(renderContext, panel);
      }
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
void Canvas3D::drawPanel(const RenderContext3D& context, Canvas* panel) {
   if (!panel->getVisible()) return;
   auto& target = renderTargetOf(panel);
   if (!target.ready()) return;
   auto size = panel->getSize();
   //counter-clockwise as seen from +Z, starting at the bottom left
   const Pos<R_FLOAT> corners[] = {{0, size.y}, {size.x, size.y}, {size.x, 0}, {0, 0}};
   rlPushMatrix();
   rlMultMatrixf(MatrixToFloat(context.getTransform().matrix));
   rlDisableBackfaceCulling(); //so the panel can be seen from behind too
   rlSetTexture(target.getTexture().id);
   rlBegin(RL_QUADS);
   rlColor4ub(255, 255, 255, 255);
   rlNormal3f(0, 0, 1);
   for (const auto& corner : corners) {
      //render targets are upside down
      rlTexCoord2f(corner.x / size.x, 1 - corner.y / size.y);
      auto nodePos = panelToNodePos(panel, corner);
      rlVertex3f(nodePos.x, nodePos.y, nodePos.z);
   }
   rlEnd();
   rlSetTexture(0);
   rlEnableBackfaceCulling();
   rlPopMatrix();
}

/////////////////////////////////////////////////////////////////////////////////////////
Pos3<R_FLOAT> Canvas3D::panelToNodePos(const Canvas* panel, const Pos<R_FLOAT>& panelPos) {
   auto planePos = panel->getLocalTransform().transform(panelPos);
   return {planePos.x / PIXELS_PER_UNIT, -planePos.y / PIXELS_PER_UNIT, 0};
}

/////////////////////////////////////////////////////////////////////////////////////////
Pos<R_FLOAT> Canvas3D::nodeToPanelPos(const Canvas* panel, const Pos3<R_FLOAT>& nodePos) {
   Pos<R_FLOAT> planePos(nodePos.x * PIXELS_PER_UNIT, -nodePos.y * PIXELS_PER_UNIT);
   return Pos<R_FLOAT>(panel->getLocalTransform().inverse().transform(planePos));
}

/////////////////////////////////////////////////////////////////////////////////////////
std::optional<R_FLOAT> Canvas3D::intersectPanelPlane(const Ray3D& nodeRay) {
   //the plane is z = 0
   if (nodeRay.direction.z == 0) return {};
   auto distance = -nodeRay.origin.z / nodeRay.direction.z;
   if (distance < 0) return {};
   return distance;
}

/////////////////////////////////////////////////////////////////////////////////////////
void Canvas3D::collectLights(Internal::Drawable3D* drawable, std::vector<const Light3D*>& lights) {
   //hidden nodes hide their descendants, so their lights are off too
   if (!drawable->_visible) return;
   if (auto isLight = drawable->getNode()->as<Light3D>()) lights.push_back(isLight.value());
   for (auto& child : drawable->getChildren()) {
      if (auto childDrawable = child->as<Internal::Drawable3D>()) {
         collectLights(childDrawable.value(), lights);
      }
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
void Canvas3D::__on_descendant_added_to_tree(TypeNode* n) {
   //assign 3D drawables to their owning canvas, as Canvas does for 2D drawables
   if (auto isDrawable = n->as<Internal::Drawable3D>()) {
      if (!isDrawable.value()->_canvas3D) {
         isDrawable.value()->_canvas3D = this;
      }
   }
   Canvas::__on_descendant_added_to_tree(n);
}

/////////////////////////////////////////////////////////////////////////////////////////
void Canvas3D::__on_descendant_removed_from_tree(TypeNode* n) {
   if (auto isNode = n->as<Node3D>()) _removeInputStatus(isNode.value());
   if (auto isDrawable = n->as<Internal::Drawable3D>()) {
      if (isDrawable.value()->_canvas3D == this) {
         isDrawable.value()->_canvas3D = nullptr;
      }
   }
   Canvas::__on_descendant_removed_from_tree(n);
}

/////////////////////////////////////////////////////////////////////////////////////////
Ray3D Canvas3D::toBackgroundRay(const Pos<float>& canvasPos) const {
   auto size = getSize();
   return GetScreenToWorldRayEx((Vector2)canvasPos, _camera3D, (int)size.x, (int)size.y);
}

/////////////////////////////////////////////////////////////////////////////////////////
Pos<float> Canvas3D::toForegroundPos(const Pos3<float>& scenePos) const {
   auto size = getSize();
   return GetWorldToScreenEx(scenePos, _camera3D, (int)size.x, (int)size.y);
}

/////////////////////////////////////////////////////////////////////////////////////////
std::optional<Hit3D> Canvas3D::pick(const Ray3D& sceneRay) const {
   std::optional<Hit3D> nearest;
   std::function<void(TypeNode*)> visit = [&](TypeNode* n) {
      auto isNode = n->as<Node3D>();
      if (!isNode) return;
      auto node = isNode.value();
      if (!node->getVisible()) return;
      auto consider = [&](R_FLOAT distance) {
         if (!nearest || distance < nearest->distance) nearest = Hit3D{node, sceneRay, distance};
      };
      if (node->getPickable()) {
         if (auto distance = node->intersectRay(sceneRay)) consider(distance.value());
      }
      for (auto& child : n->getChildren()) {
         //a panel is part of its node, as far as the mouse is concerned (like a child node it can't tell apart)
         if (auto isPanel = child->as<Canvas>()) {
            auto panel = isPanel.value();
            if (!panel->getVisible()) continue;
            auto global = node->getGlobalTransform();
            auto nodeRay = sceneRay.transform(global.inverse().matrix);
            if (auto distance = intersectPanelPlane(nodeRay)) {
               auto nodePos = nodeRay.at(distance.value());
               if (panel->getSizeRect().contains(nodeToPanelPos(panel, nodePos))) {
                  consider(nodePos.transform(global.matrix).distanceTo(sceneRay.origin));
               }
            }
         } else {
            visit(child);
         }
      }
   };
   for (auto& child : const_cast<Canvas3D*>(this)->getChildren()) visit(child);
   return nearest;
}

/////////////////////////////////////////////////////////////////////////////////////////
Handled Canvas3D::_processBackgroundInput(const InputEvent& event) {
   //2D widgets in the background layer are drawn over the scene, so they get the input first
   if (auto handled = Canvas::_processBackgroundInput(event)) return handled;

   //express the mouse in scene space, and find what's under it once so each node can tell whether it's inside
   std::optional<MouseEvent::ScopeTransformer3D> intoScene;
   if (auto mouse = event.isMouse()) {
      auto sceneRay = toBackgroundRay(mouse.value()->getLocalPos());
      auto hit = pick(sceneRay);
      _mousePick = hit ? hit->node : nullptr;
      intoScene.emplace(*mouse.value(), sceneRay, hit ? std::optional(hit->getScenePos()) : std::nullopt);
   }
   Handled handled;
   for (auto& child : getChildren()) {
      if (auto isNode = child->as<Node3D>()) {
         if ((handled = Node3D::__pass_input(isNode.value(), event))) break;
      }
   }
   _mousePick = nullptr;
   return handled;
}
