#include "Layout.h"
#include "Window.h"
#include "ReyEngine.h"
#include "ReyStarServer.h"
#include "Label.h"

using namespace std;
using namespace ReyEngine;
using namespace ReyStar;

class AStarTile : public ReyStarTileImpl<AStarTile>{
public:
    [[nodiscard]] std::vector<std::pair<int, int>> neighborCoords() const {
        return { /*square grid, diagonals allowed*/
            {coord.x - 1, coord.y - 1}, {coord.x, coord.y - 1}, {coord.x + 1, coord.y - 1},
            {coord.x - 1, coord.y},                             {coord.x + 1, coord.y},
            {coord.x - 1, coord.y + 1}, {coord.x, coord.y + 1}, {coord.x + 1, coord.y + 1},
        };
    }
    [[nodiscard]] float distanceTo(const AStarTile& other) const {
        return (float)coord.distanceTo(other.coord);
    }
};

class UIWidget : public Layout {
public:
    UIWidget() : Layout(LayoutDir::VERTICAL) {

    }
    void _init() override {
        auto hbox1 = make_child<HLayout>("hbox1");
    }
};

TileMap<AStarTile> astarTileMap;
ReyStarServer reyStarServer(astarTileMap);

class AStarWidget : public Widget {
    std::optional<Rect<float>> _highLightCell;
    std::optional<Rect<float>> _startCell;
    std::optional<Rect<float>> _endCell;
    std::optional<std::future<PathResult<AStarTile>>> _requestedPath;
    std::optional<PathResult<AStarTile>> _currentPath;
public:
    static constexpr float CELL_SIZE = 30;
    REYENGINE_OBJECT(AStarWidget)
    AStarWidget() {
        setProcess(true);
    }
    void render2D(RenderContext& ctx) const override {
        auto rect = getSizeRect();
        if (_highLightCell) {
            drawRectangle(_highLightCell.value(), Colors::yellow);
        }
        if (_startCell) {
            drawRectangle(_startCell.value(), Colors::green);
        }
        if (_endCell) {
            drawRectangle(_endCell.value(), Colors::blue);
        }
        for (float x = 0; x <= rect.width; x += CELL_SIZE) {
            drawLine(Line<R_FLOAT>(x, 0, x, rect.height), 1.0, Colors::gray);
        }
        for (float y = 0; y <= rect.height; y += CELL_SIZE) {
            drawLine(Line<R_FLOAT>(0, y, rect.width, y), 1.0, Colors::gray);
        }

        //draw path
        if (_currentPath) {
            for (const auto& pathTile : _currentPath.value().path) {
                if (!pathTile.nextTile) break;
                auto& thisTile = pathTile.thisTile;
                auto& nextTile = pathTile.nextTile->thisTile;
                auto a = rect.getSubRectAtCoords({CELL_SIZE, CELL_SIZE}, thisTile->coord);
                auto b = rect.getSubRectAtCoords({CELL_SIZE, CELL_SIZE}, nextTile->coord);
                drawLine({a.center(), b.center()}, 2.0, Colors::red);
            }
        }
    }

private:
    void _process(float delta_ms) override {
        if (_requestedPath) {
            auto& future = _requestedPath.value();
            if (future.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
                //path is ready
                auto path = future.get();
                if (path.pathFound) {
                    _currentPath = std::move(path);
                }
                _requestedPath.reset();
            }
        }

    }
    Handled _unhandled_input(const InputEvent& e) override {
        std::optional<Rect<float>> mouseCell;
        if (e.isMouse()) {
            mouseCell = getSizeRect().getSubRectAtPos({CELL_SIZE, CELL_SIZE}, e.isMouse().value()->getLocalPos());
        }
        switch (e.eventId) {
        default: break;
        case InputEventMouseMotion::ID: {
            const auto& mmEvent = e.toEvent<InputEventMouseMotion>();
            if (mmEvent.mouse.isInside()) {
                _highLightCell = mouseCell;
            }
            break;}
        case InputEventMouseButton::ID: {
            const auto& mbEvent = e.toEvent<InputEventMouseButton>();
            if (mbEvent.isDown) break;
            if (mbEvent.mouse.isInside()){
                switch (mbEvent.button) {
                case InputInterface::MouseButton::LEFT:{
                    auto oldStart = _startCell;
                    auto oldEnd = _endCell;
                    bool needsNewPath = false;
                    if (!_startCell) {
                        _startCell = mouseCell;
                    } else {
                        _endCell = mouseCell;
                    }
                    if (_startCell && _endCell) {
                        if (_endCell) needsNewPath = _startCell.value() != _endCell.value();
                    }
                    _currentPath.reset();
                    if (needsNewPath) {
                        Pos<int> startCoord(_startCell.value().pos() / CELL_SIZE);
                        Pos<int> endCoord(_endCell.value().pos() / CELL_SIZE);
                        auto startIt = astarTileMap.find({startCoord.x, startCoord.y});
                        auto endIt = astarTileMap.find({endCoord.x, endCoord.y});
                        if (startIt != astarTileMap.end() && endIt != astarTileMap.end()) {
                            //pass references to the actual tiles living in astarTileMap - the server
                            //processes requests asynchronously and compares tiles by pointer identity,
                            //so temporaries here would both dangle and never match the goal
                            _requestedPath = reyStarServer.requestPath(startIt->second, endIt->second);
                        }
                    }
                    return this;}
                case InputInterface::MouseButton::RIGHT:
                    _startCell.reset();
                    _endCell.reset();
                    _currentPath.reset();
                    _requestedPath.reset();
                    return this;
                default: break;
                }
            }
        break;}
        }
        return nullptr;
    }
};



int main(int argc, char** argv){
    constexpr auto screenSize = Size{1920,1080} - Size(100,100);
    auto& window = Application::createWindowPrototype("AStarServer", screenSize.x, screenSize.y, {WindowFlags::RESIZE}, 60)->createWindow();
    auto root = window.getCanvas();

    //populate the tile map with a grid of walkable tiles so the server has something to path through
    auto gridWidth = (int)(screenSize.x / AStarWidget::CELL_SIZE) + 1;
    auto gridHeight = (int)(screenSize.y / AStarWidget::CELL_SIZE) + 1;
    for (int x = 0; x < gridWidth; x++) {
        for (int y = 0; y < gridHeight; y++) {
            AStarTile tile;
            tile.coord = Pos<int>(x, y);
            tile.weight = 1.0f;
            astarTileMap[{x, y}] = tile;
        }
    }

    auto astarWidget = root->make_child<AStarWidget>("astar");
    astarWidget->setAnchoring(Anchor::FILL);

    auto ui = astarWidget->make_child<UIWidget>("ui");
    ui->setAnchoring(Anchor::EVENT);

    auto on_anchor = [&](const Widget::EventAnchoring& e) {
        constexpr Size<float> widgetSize = {200, 200};
        constexpr float margin = 20;
        auto topRight = getScreenSize().toRect().topRight();
        e.rect.setSize(widgetSize);
        e.rect.setPos(topRight + Pos<float>(-margin - widgetSize.x, margin));
    };
    ui->subscribe<Widget::EventAnchoring>(ui, on_anchor);

    auto on_rect_changed = [&](const Widget::RectChangedEvent& e) {
        Logger::debug() << "ui rect = " << ui->getRect() << endl;
    };
    ui->subscribe<Widget::RectChangedEvent>(ui, on_rect_changed);

    window.exec();
    return 0;
}