#include "ReyEngine3D.h"

#include "Application.h"
#include "Window.h"
#include "Canvas3D.h"
#include "MeshInstance3D.h"
#include "Light3D.h"
#include "Label.h"
#include "Button.h"

using namespace ReyEngine;

// 3D nodes handle input the way widgets do: override _unhandled_input and return Handled{this} to take the event.
// Clicking one of these cycles its color.
class ColorCycler : public MeshInstance3D {
public:
    REYENGINE_OBJECT(ColorCycler)
    using MeshInstance3D::MeshInstance3D;
protected:
    Handled _unhandled_input(const InputEvent& e) override {
        auto click = e.isEvent<InputEventMouseButton>();
        if (!click || !click.value()->isDown || !click.value()->mouse.isInside()) return nullptr;
        static const ColorRGBA palette[] = {Colors::red, Colors::blue, Colors::green, Colors::yellow, Colors::gray};
        // advance from whatever color we have now, so the first click always changes it
        auto current = getMaterial()->getColor();
        size_t next = 0;
        for (size_t i = 0; i < std::size(palette); i++) {
            if (palette[i] == current) {next = i + 1; break;}
        }
        getMaterial()->setColor(palette[next % std::size(palette)]);
        return {this};
    }
};

int main(){
    auto& window = Application::createWindowPrototype("3D Test", 800, 600, {WindowFlags::RESIZE}, 60)->createWindow();
    auto root = window.getCanvas();

    auto canvas3d = make_child<Canvas3D>(root, "canvas3d");
    canvas3d->setAnchoring(Anchor::FILL);
    canvas3d->getCamera3D().position = {4, 3, 5};

    // meshes are shared, so every cube below draws the same GPU mesh
    auto cubeMesh = ReyMesh::makeCube();
    auto floor = make_child<ColorCycler>(canvas3d, "floor", cubeMesh, Colors::gray);
    floor->setPosition(0, -0.75, 0);
    floor->setScale({8, 0.5, 8});

    auto cube = make_child<ColorCycler>(canvas3d, "cube", cubeMesh, Colors::red);
    auto sphere = make_child<ColorCycler>(canvas3d, "sphere", ReyMesh::makeSphere(0.5f), Colors::blue);
    sphere->setPosition(2, 0, 0);

    // children inherit their parent's transform, as in 2D. The moon doesn't handle clicks, so (as in 2D) they go
    // on to its parent: clicking the moon cycles the sphere's color.
    auto moon = make_child<MeshInstance3D>(sphere, "moon", ReyMesh::makeSphere(0.15f), Colors::white);
    moon->setPosition(0, 0.8, 0);

    auto suzanne = make_child<ModelInstance3D>(canvas3d, "suzanne", FileSystem::File("test/suzanne.obj"));
    suzanne->setPosition(-2, 0, 0);
    suzanne->setScale(0.6);
    suzanne->setTint(Colors::green);

    // spin the cube using the same process callbacks widgets use
    cube->setProcess(std::function<void()>([cube](){ cube->rotate({0, 1, 0}, Radians(GetFrameTime())); }));

    // a 2D canvas under a 3D node is drawn as a panel in the node's XY plane, so it spins with the cube. The face node
    // puts the panel's top left corner on the corner of the cube's front face (just in front of it, so the cube doesn't
    // hide it) and scales it so its 200 pixels span the face's one unit (Canvas3D::PIXELS_PER_UNIT is 100).
    auto cubeFace = make_child<Node3D>(cube, "cubeFace");
    cubeFace->setPosition(-0.5, 0.5, 0.501);
    cubeFace->setScale(0.5);
    auto panel = make_child<Canvas>(cubeFace, "panel");
    panel->setSize(200, 200);
    auto clicks = make_child<Label>(panel, "clicks", "clicks: 0");
    clicks->setPosition(20, 30);
    // the button takes the click, so clicking it doesn't change the cube's color. Clicking elsewhere on the panel does,
    // since the panel doesn't handle it and it goes on to the cube.
    auto button = make_child<PushButton>(panel, "button", "click me");
    button->setRect(20, 100, 160, 60);
    clicks->subscribe<PushButton::ButtonPressEvent>(button.get(), [label = clicks.get(), count = 0](const PushButton::ButtonPressEvent&) mutable {
        label->setText("clicks: " + std::to_string(++count));
    });

    canvas3d->setAmbientLight({30, 30, 40, 255});
    make_child<DirectionalLight3D>(canvas3d, "sun", Vec3<float>{-1, -2, -1}, Colors::white, 0.6f);
    auto lamp = make_child<PointLight3D>(canvas3d, "lamp", Colors::yellow, 1.5f, 6.0f);
    lamp->setPosition(1, 1.5f, 1.5f);

    // 2D widgets added to a Canvas3D are drawn over the 3D scene, like a HUD
    auto hud = make_child<Label>(canvas3d, "hud", "ReyEngine 3D");
    hud->setPosition(10, 10);
    hud->setProcess(std::function<void()>([hud, canvas3d](){
        auto hovered = canvas3d->getHover();
        hud->setText(hovered ? "hovering " + hovered->getName() : "ReyEngine 3D - click a mesh");
    }));

    window.exec();
    return 0;
}
