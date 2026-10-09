#pragma once
#include <vector>
#include "Canvas.h"

namespace ReyEngine {

// Simple 3D camera wrapper (named to avoid clashing with raylib's global Camera3D)
struct ReyCamera3D {
    Vec3<float> position{0,0,5};
    Vec3<float> target{0,0,0};
    Vec3<float> up{0,1,0};
    float fovy = 45.0f; // degrees
};

struct Light3D {
    enum class Type {DIRECTIONAL, POINT};
    Type type = Type::DIRECTIONAL;
    Vec3<float> vector{0,-1,0}; // direction the light travels for DIRECTIONAL, world position for POINT
    ColorRGBA color = Colors::white;
    float intensity = 1.0f;
    float range = 0.0f; // POINT only. Light fades to nothing at this distance. 0 = no falloff
    bool enabled = true;
};

class Canvas3D : public Canvas {
public:
    static constexpr size_t MAX_LIGHTS = 8;
    REYENGINE_OBJECT(Canvas3D)
    Canvas3D() = default;
    ~Canvas3D() override;
    // expose camera
    ReyCamera3D& getCamera3D(){return _camera3D;}
    const ReyCamera3D& getCamera3D() const {return _camera3D;}

    // simple 3D drawing helpers. Shapes are retained and drawn every frame until cleared.
    void drawCube(const Vec3<float>& pos, const Vec3<float>& size, const ColorRGBA& color);
    void drawSphere(const Vec3<float>& pos, float radius, const ColorRGBA& color);
    void clearShapes(){_shapes.clear();}

    // lighting. With no lights, shapes are drawn unlit (flat color).
    // add* return the light's index, which can be used with getLight. Throws if MAX_LIGHTS is exceeded.
    size_t addDirectionalLight(const Vec3<float>& direction, const ColorRGBA& color = Colors::white, float intensity = 1.0f);
    size_t addPointLight(const Vec3<float>& position, const ColorRGBA& color = Colors::white, float intensity = 1.0f, float range = 0.0f);
    Light3D& getLight(size_t index){return _lights.at(index);}
    const std::vector<Light3D>& getLights() const {return _lights;}
    void clearLights(){_lights.clear();}
    void setAmbientLight(const ColorRGBA& color){_ambient = color;}
    const ColorRGBA& getAmbientLight() const {return _ambient;}

protected:
    void renderProcess(RenderContext& ctx) override;

private:
    struct Shape {
        enum class Type {CUBE, SPHERE} type;
        Vec3<float> pos;
        Vec3<float> size; // for spheres, size.x is the radius
        ColorRGBA color;
    };
    void loadGpuResources();
    void uploadLightUniforms();

    ReyCamera3D _camera3D;
    std::vector<Shape> _shapes;
    std::vector<Light3D> _lights;
    ColorRGBA _ambient = {40, 40, 40, 255};

    // GPU resources, created lazily on first render since they need a GL context
    bool _gpuLoaded = false;
    Mesh _cubeMesh{};   // unit cube, scaled per shape
    Mesh _sphereMesh{}; // unit radius sphere, scaled per shape
    Material _litMaterial{};
    Material _unlitMaterial{};
    struct {
        int viewPos, ambient, lightCount, lightType, lightVector, lightColor, lightRange;
    } _locs{};
};

} // namespace ReyEngine
