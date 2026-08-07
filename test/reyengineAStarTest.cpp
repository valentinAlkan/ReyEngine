#include "Window.h"
#include "ReyEngine.h"
#include "ReyStarServer.h"

using namespace std;
using namespace ReyEngine;
using namespace ReyStar;

class AStarTile : public ReyStarTileImpl<AStarTile>{
public:
    [[nodiscard]] std::vector<std::pair<int, int>> neighborCoords() const { return {}; }
    [[nodiscard]] float distanceTo(const AStarTile& other) const { return 0; }
};

class AStarWidget : Widget {
    bool dirty = false;
public:
    static constexpr float CELL_SIZE = 30;
    REYENGINE_OBJECT(AStarWidget)
    AStarWidget(){}
    void render2D(RenderContext& ctx) const override {
        if (dirty)
        {
            drawGrid(ctx);
        }
    }
    void render2DEnd(RenderContext&) override {
        dirty = false;
    }

    void drawGrid(RenderContext&) const
    {
        auto rect = getSizeRect();
        for (float x = 0; x <= rect.width; x += CELL_SIZE) {
            drawLine(Line<R_FLOAT>(x, 0, x, rect.height), 1.0, Colors::gray);
        }
        for (float y = 0; y <= rect.height; y += CELL_SIZE) {
            drawLine(Line<R_FLOAT>(0, y, rect.width, y), 1.0, Colors::gray);
        }
    }
};


TileMap<AStarTile> astarTileMap;
ReyStarServer<AStarTile> reyStarServer(astarTileMap);
int main(int argc, char** argv){
    constexpr auto screenSize = Size{1920,1080} - Size(100,100);
    auto& window = Application::createWindowPrototype("AStarServer", screenSize.x, screenSize.y, {WindowFlags::RESIZE}, 60)->createWindow();
    auto root = window.getCanvas();

    auto astarWidget = root->make_child<AStarWidget>("astar");
    window.exec();
    return 0;
}