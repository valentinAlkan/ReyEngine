#include "ReyEngine3D.h"

#include "Application.h"
#include "Window.h"
#include "Canvas3D.h"
#include "MeshInstance3D.h"
#include "Light3D.h"
#include "Label.h"

using namespace ReyEngine;

int main(){
    auto& window = Application::createWindowPrototype("3D Test", 800, 600, {WindowFlags::RESIZE}, 60)->createWindow();
    auto root = window.getCanvas();

    auto canvas3d = make_child<Canvas3D>(root, "canvas3d");
    canvas3d->setAnchoring(Anchor::FILL);
    canvas3d->getCamera3D().position = {4, 3, 5};

    // meshes are shared, so every cube below draws the same GPU mesh
    auto cubeMesh = ReyMesh::makeCube();
    auto floor = make_child<MeshInstance3D>(canvas3d, "floor", cubeMesh, Colors::gray);
    floor->setPosition(0, -0.75, 0);
    floor->setScale({8, 0.5, 8});

    auto cube = make_child<MeshInstance3D>(canvas3d, "cube", cubeMesh, Colors::red);
    auto sphere = make_child<MeshInstance3D>(canvas3d, "sphere", ReyMesh::makeSphere(0.5f), Colors::blue);
    sphere->setPosition(2, 0, 0);

    // children inherit their parent's transform, as in 2D
    auto moon = make_child<MeshInstance3D>(sphere, "moon", ReyMesh::makeSphere(0.15f), Colors::white);
    moon->setPosition(0, 0.8, 0);

    auto suzanne = make_child<ModelInstance3D>(canvas3d, "suzanne", FileSystem::File("test/suzanne.obj"));
    suzanne->setPosition(-2, 0, 0);
    suzanne->setScale(0.6);
    suzanne->setTint(Colors::green);

    // spin the cube using the same process callbacks widgets use
    cube->setProcess(std::function<void()>([cube](){ cube->rotate({0, 1, 0}, Radians(GetFrameTime())); }));

    canvas3d->setAmbientLight({30, 30, 40, 255});
    make_child<DirectionalLight3D>(canvas3d, "sun", Vec3<float>{-1, -2, -1}, Colors::white, 0.6f);
    auto lamp = make_child<PointLight3D>(canvas3d, "lamp", Colors::yellow, 1.5f, 6.0f);
    lamp->setPosition(1, 1.5f, 1.5f);

    // 2D widgets added to a Canvas3D are drawn over the 3D scene, like a HUD
    auto hud = make_child<Label>(canvas3d, "hud", "ReyEngine 3D");
    hud->setPosition(10, 10);

    window.exec();
    return 0;
}
