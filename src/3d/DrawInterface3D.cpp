#include "DrawInterface3D.h"
#include "Logger.h"

using namespace std;
using namespace ReyEngine;

/////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
void CameraTransform3D::push() const {
   BeginMode3D(camera);
}

/////////////////////////////////////////////////////////////////////////////////////////
CameraTransform3D::CameraTransform3D() {
   camera.position = { 10.0f, 10.0f, 10.0f }; // Camera position
   camera.target = { 0.0f, 0.0f, 0.0f };      // Camera looking at point
   camera.up = { 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
   camera.fovy = 45.0f;                                // Camera field-of-view Y
   camera.projection = CAMERA_PERSPECTIVE;             // Camera projection type
}
/////////////////////////////////////////////////////////////////////////////////////////
void CameraTransform3D::pop(){
   EndMode3D();
}
/////////////////////////////////////////////////////////////////////////////////////////
void CameraTransform3D::setFOV(float newFOV) {
   camera.fovy = newFOV;
}

/////////////////////////////////////////////////////////////////////////////////////////
Collisions::Ray3D ReyEngine::CameraTransform3D::getRay(const Pos<R_FLOAT>& pos) const {
    return GetMouseRay((Vector2)pos, camera);
}

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
Model3D::~Model3D() {
    if (isReady()) {
        UnloadModel(model);
    }
}
/////////////////////////////////////////////////////////////////////////////////////////
std::optional<std::shared_ptr<Model3D>> Model3D::load(const FileSystem::Path &path) noexcept {
    if (!path.exists()){
        Logger::warn() << "Path " + path.str() + " does not exist!" << endl;
        return nullopt;
    }
    auto thiz = shared_ptr<Model3D>(new Model3D(path));
    if (!thiz->isReady()){
        return nullopt;
    }
    return thiz;
}
/////////////////////////////////////////////////////////////////////////////////////////
std::optional<std::shared_ptr<Model3D>> Model3D::loadNewFromThis() noexcept{
    return load(filePath);
}
/////////////////////////////////////////////////////////////////////////////////////////
Model3D::Model3D(const FileSystem::Path &path){
    if (path.exists()){
        model = LoadModel(path.str().c_str());
    }
}

/////////////////////////////////////////////////////////////////////////////////////////
Collisions::RayHit3D Model3D::collides(const Collisions::Ray3D &ray) {
    std::vector<Collisions::RayHit3D> hits;
    for (const auto& mesh : getMeshes()){
        auto optHit = Collisions::collides(ray, mesh);
        if (!optHit) continue;
        const auto& hit = optHit.value();
        if (hits.empty()) {
            hits.push_back(hit);
        } else {
            //don't bother storing if its less than our existing hit
            if (hits.back().distance > hit.distance) continue;
            hits.push_back(hit);
        }
    }
    return hits.back();
}

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
bool Collisions::collides(const Sphere &a, const Sphere &b) {
    return CheckCollisionSpheres(a.center, (float)a.radius, b.center, (float)b.radius);
}
/////////////////////////////////////////////////////////////////////////////////////////
bool Collisions::collides(const BoundingBox3D &b, const Sphere& s) {
    const BoundingBox& bb = reinterpret_cast<const BoundingBox*>(&b)[Internal::OFFSET::BB_OFF]; //bb is private
    return CheckCollisionBoxSphere(bb, s.center, (float)s.radius);
}
/////////////////////////////////////////////////////////////////////////////////////////
bool Collisions::collides(const BoundingBox3D& a, const BoundingBox3D& b) {
    const BoundingBox& abb = reinterpret_cast<const BoundingBox*>(&a)[Internal::OFFSET::BB_OFF]; //bb is private
    const BoundingBox& bbb = reinterpret_cast<const BoundingBox*>(&b)[Internal::OFFSET::BB_OFF]; //bb is private
    return CheckCollisionBoxes(abb, bbb);
}
/////////////////////////////////////////////////////////////////////////////////////////
optional<Collisions::RayHit3D> Collisions::collides(const Ray3D& ray, const Mesh& mesh) {
    auto collision = GetRayCollisionMesh(ray, mesh, {});
    if (!collision.hit) return nullopt;
    return RayHit3D(ray, collision);
}

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
bool Sphere::collides(const Sphere &other) {return Collisions::collides(*this, other);}
bool Sphere::collides(const BoundingBox3D &other) {return Collisions::collides(other, *this);}
bool BoundingBox3D::collides(const BoundingBox3D& other) {return Collisions::collides(other, *this);}
bool BoundingBox3D::collides(const Sphere& other) {return Collisions::collides(*this, other);}
