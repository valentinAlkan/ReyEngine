#include "Application.h"
#include "Window.h"
#include "Canvas3D.h"

using namespace ReyEngine;

int main(){
    auto& window = Application::createWindowPrototype("3D Test", 800, 600, {WindowFlags::RESIZE}, 60)->createWindow();
    auto root = window.getCanvas();

    auto canvas3d = make_child<Canvas3D>(root, "canvas3d");
    canvas3d->setAnchoring(Anchor::FILL);
    canvas3d->getCamera3D().position = {4, 3, 5};

    canvas3d->drawCube({0,-0.75,0}, {8,0.5,8}, Colors::gray); // floor
    canvas3d->drawCube({0,0,0}, {1,1,1}, Colors::red);
    canvas3d->drawSphere({2,0,0}, 0.5f, Colors::blue);

    canvas3d->setAmbientLight({30, 30, 40, 255});
    canvas3d->addDirectionalLight({-1, -2, -1}, Colors::white, 0.6f);
    canvas3d->addPointLight({1, 1.5f, 1.5f}, Colors::yellow, 1.5f, 6.0f);

    window.exec();
    return 0;
}
