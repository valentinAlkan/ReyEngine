#include "reyTestMain.h"
#include "DrawInterface.h"
#include <cassert>
#include "ScrollArea.h"
#include "Application.h"
#include <iostream>
#include "Logger.h"
#include "ArgParse.h"
#include "Label.hpp"
#include "Button.h"
#include "RootWidget.hpp"
#include "TextureTestWidget.hpp"
#include "Slider.hpp"
#include "Timer.hpp"
#include "Layout.h"
#include "Panel.h"
#include "Editor.h"
#include "Tree.h"
#include "Inspector.h"
#include "Sprite.h"
#include "Canvas.h"
#include "TabContainer.h"
#include "ComboBox.h"
#include "LineEdit.h"
#include "Config.h"
#include "XML.h"
#include "TileMap.h"
#include "TextureRect.h"
#include "AStar.h"
#include "CSVParser.h"
#include "Camera2D.h"
#include "Viewport.h"
#include "ModelBody.h"
#include "Geodetic.h"

using namespace std;
using namespace ReyEngine;
using namespace PathFinding;

int screenWidth = 1500;
int screenHeight = 800;

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv) {
    {
        SetTraceLogLevel(LOG_ERROR);
#define HOWSIZE(CLASSNAME) Logger::debug() << "Type " << #CLASSNAME << " = " << sizeof(CLASSNAME) << " bytes" << endl;

        HOWSIZE(Internal::TypeContainer<Internal::NamedInstance>)
        HOWSIZE(Internal::TypeContainer<Internal::Component>)
        HOWSIZE(Internal::TypeContainer<BaseWidget>)
        HOWSIZE(inheritable_enable_shared_from_this<Internal::Component>)
        HOWSIZE(PropertyMap)
        HOWSIZE(OwnedPropertyMap)
        HOWSIZE(PropertyContainer)
        HOWSIZE(BoolProperty)
        HOWSIZE(IntProperty)
        HOWSIZE(Internal::Component);
        HOWSIZE(BaseWidget)
        HOWSIZE(Layout)
        HOWSIZE(HLayout)
        HOWSIZE(VLayout)
        HOWSIZE(GridLayout)
        HOWSIZE(Control)
        HOWSIZE(Label)

        Application::instance(); //initialize the application
        ArgParse args;
        args.defineArg(RuntimeArg("--loadScene", "help", 1, RuntimeArg::ArgType::POSITIONAL));
        args.defineArg(RuntimeArg("--eventTest", "help", 0, RuntimeArg::ArgType::FLAG));
        args.defineArg(RuntimeArg("--renderTest", "help", 0, RuntimeArg::ArgType::FLAG));
        args.defineArg(RuntimeArg("--scrollAreaTest", "help", 0, RuntimeArg::ArgType::FLAG));
        args.defineArg(RuntimeArg("--sliderTest", "help", 0, RuntimeArg::ArgType::FLAG));
        args.defineArg(RuntimeArg("--labelTest", "help", 0, RuntimeArg::ArgType::FLAG));
        args.defineArg(RuntimeArg("--saveLoadSceneTest", "Filename to save/load to", 1, RuntimeArg::ArgType::POSITIONAL));
        args.defineArg(RuntimeArg("--xmlTest", "XML test", 0, RuntimeArg::ArgType::FLAG));
        args.defineArg(RuntimeArg("--csvTest", "CSV test", 0, RuntimeArg::ArgType::FLAG));
        args.defineArg(RuntimeArg("--layoutTest", "Test layouts", 0, RuntimeArg::ArgType::FLAG));
        args.defineArg(RuntimeArg("--configTest", "Config file test", 0, RuntimeArg::ArgType::FLAG));
        args.defineArg(RuntimeArg("--layoutTestBasic", "Basic Test layouts", 0, RuntimeArg::ArgType::FLAG));
        args.defineArg(RuntimeArg("--layoutTestSuperBasic", "Super basic Test layouts", 0, RuntimeArg::ArgType::FLAG));
        args.defineArg(RuntimeArg("--layoutTestConstrained", "Super basic Test layouts", 0, RuntimeArg::ArgType::FLAG));
        args.defineArg(RuntimeArg("--panelTest", "Test panel", 0, RuntimeArg::ArgType::FLAG));
        args.defineArg(RuntimeArg("--editor", "Editor", 0, RuntimeArg::ArgType::FLAG));
        args.defineArg(RuntimeArg("--treeTest", "TreeTest", 0, RuntimeArg::ArgType::FLAG));
        args.defineArg(RuntimeArg("--childBoundingBoxTest", "ChildBoundingBoxTest", 0, RuntimeArg::ArgType::FLAG));
        args.defineArg(RuntimeArg("--inputPositionTest", "InputPositionTest", 0, RuntimeArg::ArgType::FLAG));
        args.defineArg(RuntimeArg("--inspector", "InspectorTest", 0, RuntimeArg::ArgType::FLAG));
        args.defineArg(RuntimeArg("--spriteTest", "SpriteTest", 0, RuntimeArg::ArgType::FLAG));
        args.defineArg(RuntimeArg("--buttonTest", "PushButton usage example", 0, RuntimeArg::ArgType::FLAG));
        args.defineArg(RuntimeArg("--anchorTest", "Anchoring options test", 0, RuntimeArg::ArgType::FLAG));
        args.defineArg(RuntimeArg("--marginsTest", "Layout margins test", 0, RuntimeArg::ArgType::FLAG));
        args.defineArg(RuntimeArg("--hoverTest", "Hovering test", 0, RuntimeArg::ArgType::FLAG));
        args.defineArg(RuntimeArg("--tabContainerTest", "Tab container test", 0, RuntimeArg::ArgType::FLAG));
        args.defineArg(RuntimeArg("--dragTest", "Dragging and relative movement test", 0, RuntimeArg::ArgType::FLAG));
        args.defineArg(RuntimeArg("--drawTest", "Test various drawing functions", 0, RuntimeArg::ArgType::FLAG));
        args.defineArg(RuntimeArg("--circleEstimateTest", "Test drawing big circles", 0, RuntimeArg::ArgType::FLAG));
        args.defineArg(RuntimeArg("--comboBoxTest", "Combo box test", 0, RuntimeArg::ArgType::FLAG));
        args.defineArg(RuntimeArg("--tileMapTest", "Config file test", 0, RuntimeArg::ArgType::FLAG));
        args.defineArg(RuntimeArg("--lineEditTest", "Line edit test", 0, RuntimeArg::ArgType::FLAG));
        args.defineArg(RuntimeArg("--canvasTest", "Testing nested canvases to make sure they work right", 0, RuntimeArg::ArgType::FLAG));
        args.defineArg(RuntimeArg("--scissorTest", "scissoring test", 0, RuntimeArg::ArgType::FLAG));
        args.defineArg(RuntimeArg("--uniqueValueTest", "unique value test", 0, RuntimeArg::ArgType::FLAG));
        args.defineArg(RuntimeArg("--readFileTest", "char reading test", 0, RuntimeArg::ArgType::FLAG));
        args.defineArg(RuntimeArg("--astarTest", "a star test", 0, RuntimeArg::ArgType::FLAG));
        args.defineArg(RuntimeArg("--astarTestAuto", "a star test", 0, RuntimeArg::ArgType::FLAG));
        args.defineArg(RuntimeArg("--camera2dTest", "camera 2d test", 0, RuntimeArg::ArgType::FLAG));
        args.defineArg(RuntimeArg("--3dTest", "Basic3DTest", 0, RuntimeArg::ArgType::FLAG));
        args.defineArg(RuntimeArg("--freeTest", "Resource freeing test", 0, RuntimeArg::ArgType::FLAG));
        args.defineArg(RuntimeArg("--rotationTest", "Test of rotation 2dpositionables", 0, RuntimeArg::ArgType::FLAG));
        args.parseArgs(argc, argv);

        //create window (or don't idk)
        auto windowProto = Application::instance().createWindowPrototype("MainWindow", screenWidth, screenHeight,
                                                                         {Window::RESIZE});
        auto &window = windowProto.createWindow();
        auto root = window.getCanvas();

        auto argLoadScene = args.getArg("--loadScene");
        if (argLoadScene) {
            auto loadSceneArg = argLoadScene.value();
            //open and test a scene file
            auto loadedScene = Scene::fromFile("./test/" + loadSceneArg->getParams()[0]);
            if (loadedScene) {
                Logger::debug() << "Got loaded file!" << endl;
                throw std::runtime_error("--loadscene test: fix me");
    //         root->addChild(loadedScene.value()->getRoot());
            }
        } else if (args.getArg("--eventTest")) {
            auto sprite = Sprite::build("sprite");
            static int j = 0;
            auto cb = [&](const InputEventMouseMotion &event) {
                std::cout << "got InputEvent" << j << endl;
                j++;
            };
            static constexpr int PUB_COUNT = 10;
            for (int i = 0; i < PUB_COUNT; i++) {
                sprite->subscribe<InputEventMouseMotion>(sprite, cb);
            }

            for (int i = 0; i < 10; i++) {
                InputEventMouseMotion e(sprite);
                sprite->publish<InputEventMouseMotion>(e);
                if (j != PUB_COUNT) {
                    throw std::runtime_error("Did not catch 10 events");
                }
                j = 0;
            }
        } else if (args.getArg("--drawTest")) {
            auto control = Control::build("Drawbox");
            control->setAnchoring(BaseWidget::Anchor::FILL);
            root->addChild(control);

            static constexpr int GRID_SIZE = 50;
            static constexpr Rect<int> GRID = {0, 0, 1920, 1080};
            auto drawcb = [control](const Control &ctl) {
                auto getDrawCell = [](int index) {
                    return Rect<int>(getScreenSize()).getSubRect(Size<int>(GRID_SIZE), index);
                };
                auto getDrawCellRange = [](int indexstart, int indexstop) {
                    return Rect<int>(getScreenSize()).getSubRect(Size<int>(GRID_SIZE), indexstart, indexstop);
                };

                ReyEngine::drawRectangle(getDrawCell(0), Colors::red);
                {
                    float startang = 0;
                    static int endang = 0;
                    auto cell = getDrawCell(1);
                    auto circle = cell.inscribe();
                    auto sector = CircleSector(circle, startang, endang++ % 360);
                    ReyEngine::drawCircleSectorLines(sector, Colors::blue, 20);
                }
                {
                    //draw diagonal overlapping rectangles
                    auto rect0 = getDrawCellRange(2, 33);
                    auto rect1 = getDrawCellRange(5, 36);
                    auto rect2 = getDrawCellRange(92, 123);
                    auto rect3 = getDrawCellRange(95, 126);
                    auto rectMiddle = getDrawCellRange(33, 95);
                    ReyEngine::drawRectangle(rect0, Colors::yellow);
                    ReyEngine::drawRectangle(rect1, Colors::yellow);
                    ReyEngine::drawRectangle(rect2, Colors::yellow);
                    ReyEngine::drawRectangle(rect3, Colors::yellow);
                    //draw a central rectangle
                    ReyEngine::drawRectangle(rectMiddle, Colors::blue);

                    //now draw the overlaps
                    ReyEngine::drawRectangle(rect0.getOverlap(rectMiddle), Colors::green);
                    ReyEngine::drawRectangle(rect1.getOverlap(rectMiddle), Colors::green);
                    ReyEngine::drawRectangle(rect2.getOverlap(rectMiddle), Colors::green);
                    ReyEngine::drawRectangle(rect3.getOverlap(rectMiddle), Colors::green);
                }

                {
                    //draw orthogonal overlaps
                    {
                        // 2
                        auto rect0 = getDrawCellRange(100, 161);
                        auto rect1 = getDrawCellRange(131, 132);
                        ReyEngine::drawRectangle(rect0, Colors::blue);
                        ReyEngine::drawRectangle(rect1, Colors::yellow);
                        ReyEngine::drawRectangle(rect0.getOverlap(rect1), Colors::green);
                    }
                    {
                        //form 3
                        auto rect0 = getDrawCellRange(98, 159);
                        auto rect1 = getDrawCellRange(127, 128);
                        ReyEngine::drawRectangle(rect0, Colors::yellow);
                        ReyEngine::drawRectangle(rect1, Colors::blue);
                        ReyEngine::drawRectangle(rect0.getOverlap(rect1), Colors::green);
                    }
                    {
                        auto rect0 = getDrawCellRange(13, 45);
                        auto rect1 = getDrawCellRange(44, 74);
                        ReyEngine::drawRectangle(rect0, Colors::blue);
                        ReyEngine::drawRectangle(rect1, Colors::yellow);
                        ReyEngine::drawRectangle(rect0.getOverlap(rect1), Colors::green);
                    }
                    {
                        auto rect0 = getDrawCellRange(133, 165);
                        auto rect1 = getDrawCellRange(104, 134);
                        ReyEngine::drawRectangle(rect0, Colors::blue);
                        ReyEngine::drawRectangle(rect1, Colors::yellow);
                        ReyEngine::drawRectangle(rect0.getOverlap(rect1), Colors::green);
                    }
                }

                {
                    {
                        //draw entirely overlapping rectangles
                        auto rect1 = getDrawCellRange(7, 69);
                        auto rect2 = getDrawCell(38);
                        //draw overlap
                        ReyEngine::drawRectangle(rect1, Colors::red);
                        ReyEngine::drawRectangle(rect2, Colors::blue);
                        ReyEngine::drawRectangle(rect1.getOverlap(rect2), Colors::purple);
                    }
                    {
                        //and the opposite way
                        auto rect1 = getDrawCellRange(10, 72);
                        auto rect2 = getDrawCell(41);
                        //draw overlap
                        ReyEngine::drawRectangle(rect1, Colors::yellow);
                        ReyEngine::drawRectangle(rect2, Colors::red);
                        ReyEngine::drawRectangle(rect2.getOverlap(rect1), Colors::orange);
                    }
                }

                //draw a grid
                int index = 0;
                auto screenSize = getScreenSize();
                for (int x = 0; x < screenSize.x / GRID_SIZE; x++) {
                    auto _x = x * GRID_SIZE;
                    ReyEngine::drawLine(Line<int>(_x, 0, _x, screenSize.y), 1.0, Colors::black);
                    for (int y = 0; y < screenSize.y / GRID_SIZE; y++) {
                        auto _y = y * GRID_SIZE;
                        ReyEngine::drawLine(Line<int>(0, _y, screenSize.x, _y), 1.0, Colors::black);
                        ReyEngine::drawText(to_string(index), getDrawCell(index).pos(), getDefaultFont(10));
                        index++;
                    }
                }

                //draw a rotating arrow
                {
                    auto rect = GRID.getSubRectAtCoords(Size(GRID_SIZE), Rect<int>::SubRectCoords({0, 1}));
                    Line<int> l = {rect.leftSide().midpoint(), rect.rightSide().midpoint()};
                    auto sec = (double) Application::secondsSinceInit();
                    ctl.drawArrow(l.rotate(l.midpoint(), Degrees(360) * sec % 360), 2.0, Colors::blue);
                }

                auto factor = abs(sin(Application::secondsSinceInit()));
                //draw a rotating line
                {
                    auto rect = GRID.getSubRectAtCoords(Size(GRID_SIZE), Rect<int>::SubRectCoords({1, 1}));
                    Line<int> l = {rect.leftSide().midpoint(), rect.rightSide().midpoint()};
                    ctl.drawLine(l.rotate(l.midpoint(), Degrees(360 * factor)), 2.0, Colors::blue);
                }

                //draw a rotating point
                {
                    auto rect = GRID.getSubRectAtCoords(Size(GRID_SIZE), Rect<int>::SubRectCoords({0, 2}));
                    Line<int> l = {rect.center(), rect.leftSide().midpoint()};
                    l = l.rotate(rect.center(), Degrees(360 * factor));
                    auto c1 = Circle(l.b, 2.0);
                    ctl.drawCircle(c1, Colors::blue);
                    ctl.drawLine(l.project(l.distance() * .5), 2.0, Colors::blue);
                }
            };

            control->setRenderCallback(drawcb);
        } else if (args.getArg("--circleEstimateTest")) {
            auto control = Control::build("Drawbox");
            control->setAnchoring(BaseWidget::Anchor::FILL);
            root->addChild(control);

            static constexpr ReyEngine::ColorRGBA CIRCLE_COLOR = Colors::blue;
            static constexpr ReyEngine::ColorRGBA NORMAL_COLOR = Colors::red;
            static constexpr size_t POINTS_MAX = 3;
            static constexpr int POINT_RADIUS = 3;
            static constexpr int NORMAL_LEN = 200;
            static constexpr Rect<int> GRID = {0, 0, 1920, 1080};

            vector<Pos<int>> points = {{},
                                       {},
                                       {}};
            std::optional<Pos<int>> normal;
            std::optional<Circle> circle;
            std::optional<Pos<int>> degreePoint;
            InputInterface::MouseButton btnDown = InputInterface::MouseButton::NONE;
            size_t ptIndex = 0; //the index of hte point we are dragging

            auto inputcb = [&](Control &, const InputEvent &event,
                               const std::optional<UnhandledMouseInput> &mouse) -> Handled {
                if (!mouse || !mouse.value().isInside) return false;
                switch (event.eventId) {
                    case InputEventMouseButton::getUniqueEventId(): {
                        const auto &mbEvent = event.toEventType<InputEventMouseButton>();
                        if (!mbEvent.isDown) {
                            btnDown = InputInterface::MouseButton::NONE;
                            return true;
                        }
                        btnDown = mbEvent.button;
                        ptIndex = (ptIndex + 1) % POINTS_MAX;
                    }
                        //fall through to the motion event!
                    case InputEventMouseMotion::getUniqueEventId(): {
                        if (btnDown == InputInterface::MouseButton::NONE) return false;
                        if (btnDown == InputInterface::MouseButton::LEFT) {
                            points.at(ptIndex) = mouse->localPos;
                            circle = Circle::fromPoints(points.at(0), points.at(1), points.at(2));
                            return true;
                        } else if (btnDown == InputInterface::MouseButton::RIGHT) {
                            //create a new normal point
                            normal = mouse->localPos;
                            return true;
                        }
                    }
                }
                return false;
            };

            auto drawcb = [&](const Control &ctl) {
                for (const auto &p: points) {
                    ctl.drawCircle({p, POINT_RADIUS}, CIRCLE_COLOR);
                }
                if (circle) {
                    //draw the estimated circle
                    ctl.drawCircleLines(circle.value(), CIRCLE_COLOR);
                }

                if (normal) {
                    //draw normal point
                    ctl.drawCircle({normal.value(), POINT_RADIUS}, NORMAL_COLOR);
                }

                if (normal && circle) {
                    //draw the normal line
                    const auto &normalLine = Line<int>(normal.value(), circle->center);
                    ctl.drawLine(normalLine, 1, NORMAL_COLOR);
                    //draw tangent point
                    const auto &tangentPoint = circle.value().getTangentPoint(normal.value());
                    ctl.drawCircleLines({tangentPoint, POINT_RADIUS}, NORMAL_COLOR);
                    //draw tangent line
                    const auto &tangentLine = circle.value().getTangentLine(tangentPoint, NORMAL_LEN);
                    ctl.drawLine(Line<double>(tangentLine), 1, NORMAL_COLOR);
                }

                if (degreePoint) {
                    ctl.drawCircle({degreePoint.value(), 3}, Colors::green);
                }
            };

            control->setRenderCallback(drawcb);
            control->setUnhandledInputCallback(inputcb);

            static Degrees d = 0;
            auto proc = [&](Control &ctl, double dt) {
                if (circle) {
                    d += 0.25;
                    degreePoint = circle->getPoint(d);
                } else {
                    degreePoint = nullopt;
                }
            };
            control->setProcessCallback(proc);
        } else if (args.getArg("--inputPositionTest")) {
            auto control = Control::build("MainControl");
            control->setRect(Rect<int>(40, 40, 50, 50));
            auto subcontrol = Control::build("SubControl");
            subcontrol->setRect(Rect<int>(100, 100, 50, 50));
            control->getTheme()->background.set(Style::Fill::SOLID);
            control->getTheme()->background.colorPrimary.set(COLORS::lightGray);

            auto renderSubControl = [subcontrol](const Control &) {
                auto rect = subcontrol->getGlobalRect();
                ReyEngine::drawRectangle(rect, COLORS::blue);
            };

            auto process = [&](const Control &, float dt) {
                subcontrol->setRect({{control->globalToLocal(InputManager::getMousePos())},
                                     {50, 50}});
            };

            control->addChild(subcontrol);
            subcontrol->setRenderCallback(renderSubControl);
            subcontrol->setProcessCallback(process);
            root->addChild(control);
        } else if (args.getArg("--renderTest")) {
            root->addChild(RootWidget::build("Root"));
            root->setRect({0, 0, 0, 0});
            auto textureTest = TextureTestWidget::build("TexTest");
            textureTest->setRect({0, 0, 100, 100});
            root->addChild(textureTest);
        } else if (args.getArg("--labelTest")) {
            root->addChild(RootWidget::build("Root"));
            auto testLabel = Label::build("Label");
            testLabel->setRect({0, 0, 50, 50});
            root->addChild(testLabel);
    //      testLabel->setOutlineType(Style::Outline::LINE);
    //      testLabel->setBackgroundType(Theme::Outline::LINE);
        } else if (args.getArg("--scissorTest")) {
            auto background = Control::build("background");
            auto foreground = Control::build("foreground");
            root->addChild(background);
            background->addChild(foreground);
            background->setAnchoring(BaseWidget::Anchor::FILL);
            foreground->setAnchoring(BaseWidget::Anchor::FILL);
            background->getTheme()->background.colorPrimary = ColorRGBA(0, 0, 255, 50);
            foreground->getTheme()->background.colorPrimary = ColorRGBA(0, 255, 0, 50);

            static constexpr int CURSOR_SIZE = 100;
            auto cursor = Rect<int>(Size<int>(CURSOR_SIZE));
            //set a moving scissor area
            auto foreproc = [&](Control &foreground, double dt) {
                background->setScissorArea(background->getRect().toSizeRect().embiggen(-50));
                cursor.setPos(background->getLocalMousePos());
                foreground.setScissorArea(cursor);
            };
            foreground->setProcessCallback(foreproc);
        } else if (args.getArg("--scrollAreaTest")) {
            root->addChild(Control::build("Control"));
            root->setRect({0, 0, 2000, 2000});

            //add labels
            auto labelLayout = HLayout::build("labelLayout");
            labelLayout->setRect(Rect<int>(50, 20, 350, 20));
            root->addChild(labelLayout);
            auto xlabel = Label::build("XLabel");
            xlabel->setAnchoring(BaseWidget::Anchor::CENTER);
            auto ylabel = Label::build("YLabel");
            ylabel->setAnchoring(BaseWidget::Anchor::CENTER);
            auto xslider = make_shared<Slider>("XSlider", Slider::SliderType::HORIZONTAL);
            auto yslider = make_shared<Slider>("YSlider", Slider::SliderType::HORIZONTAL);
            auto spacer = Control::build("spacer");
            labelLayout->addChild(xslider);
            labelLayout->addChild(spacer);
            labelLayout->addChild(yslider);
            labelLayout->layoutRatios = {1, .1, 1};
            xslider->addChild(xlabel);
            yslider->addChild(ylabel);

            //add scroll area
            auto scrollArea = ScrollArea::build("ScrollArea");
            scrollArea->setRect(Rect<int>(50, 50, 500, 500));
            auto label1 = Label::build("ScrollAreaLabel1");
            auto label2 = Label::build("ScrollAreaLabel2");
            label1->setRect({40, 40, 0, 0});
            label2->setRect({300, 300, 0, 0});
            label1->setText("Hello from somewhere nearish to the top left!");
            label2->setText("Hello from the bottom right!");
            scrollArea->addChild(label1);
            scrollArea->addChild(label2);
            scrollArea->getTheme()->background.colorPrimary.set(COLORS::red);

            //callback to move other labels
            auto moveLabels = [&](const Slider::EventSliderValueChanged &event) {
                const Vec2<int> range(300, 1000);
                auto newX = range.lerp(xslider->getSliderPct());
                auto newY = range.lerp(yslider->getSliderPct());
                label2->setPos({(int) newX, (int) newY});
            };

            label2->subscribe<Slider::EventSliderValueChanged>(xslider, moveLabels);
            label2->subscribe<Slider::EventSliderValueChanged>(yslider, moveLabels);

            //draw a box around the scroll area
            auto boxRect = scrollArea->getRect();
            boxRect.x -= 1;
            boxRect.y -= 1;
            boxRect.width += 1;
            boxRect.height += 1;
            auto box = Control::build("OutlineControl");
            box->setRect(boxRect);
            box->getTheme().get()->background.set(Style::Fill::SOLID);
            box->getTheme().get()->background.colorPrimary.set({125, 125, 125, 127});
            root->addChild(box);

            root->addChild(scrollArea);
    //
            auto displaySize = [&](const BaseWidget::WidgetRectChangedEvent &) {
                xlabel->setText(scrollArea->getWidth());
                ylabel->setText(scrollArea->getHeight());
                box->setRect(scrollArea->getRect());
            };
            labelLayout->subscribe<BaseWidget::WidgetRectChangedEvent>(scrollArea, displaySize);
            scrollArea->setInEditor(true);
            scrollArea->setEditorSelected(true);
        } else if (args.getArg("--childBoundingBoxTest")) {
            auto boxBounder = Control::build("BoxBounder");
            boxBounder->setRect({0, 0, 2000, 2000});
            root->addChild(boxBounder);
            //add some children
            auto label1 = Label::build("Label1");
            label1->setPos(Pos<int>(40, 40));
            boxBounder->addChild(label1);
            auto label2 = Label::build("Label2");
            label2->setPos(Pos<int>(40, 300));
            boxBounder->addChild(label2);

            //draw the child bounding box
            auto drawBoundingBox = [&](const Control &) {
                auto size = boxBounder->getChildBoundingBox();
                auto mousePos = InputManager::getMousePos();
                ReyEngine::drawRectangle({{0, 0}, size}, ReyEngine::Colors::yellow);
                ReyEngine::drawRectangle(label1->getRect(), ReyEngine::Colors::green);
                ReyEngine::drawRectangle(label2->getRect(), ReyEngine::Colors::green);
                ReyEngine::drawLine({{0, 0}, mousePos}, 1, COLORS::red);
            };
            boxBounder->setRenderCallback(drawBoundingBox);

            auto process = [&](const Control &, float dt) {
                auto globalPos = InputManager::getMousePos();
                // reposition label
                auto newPos = label1->getParent().lock()->globalToLocal(globalPos);
                label1->setRect({newPos, label1->getSize()});
            };
            boxBounder->setProcessCallback(process);

        } else if (args.getArg("--sliderTest")) {
            auto inputFilter = Control::build("InputFilter");
            inputFilter->setAnchoring(BaseWidget::Anchor::FILL);
            root->addChild(inputFilter);
            auto control = Control::build("Control");
            control->setSize({100, 100});
            control->getTheme()->background.colorPrimary = Colors::red;

            auto vslider = make_shared<Slider>("Vslider", Slider::SliderType::VERTICAL);
            vslider->setRect({200, 100, 20, 100});
            auto hslider = make_shared<Slider>("Hslider", Slider::SliderType::HORIZONTAL);
            hslider->setRect({250, 100, 100, 20});

            inputFilter->addChild(control);
            inputFilter->addChild(vslider);
            inputFilter->addChild(hslider);

            bool down = false;
            Pos<int> offset;

            auto cbInput = [&](const Control &, const InputEvent &event,
                               const std::optional<UnhandledMouseInput> &mouse) -> Handled {
                switch (event.eventId) {
                    case InputEventMouseButton::getUniqueEventId(): {
                        auto mbEvent = event.toEventType<InputEventMouseButton>();
                        if (mbEvent.isDown && mouse->isInside) {
                            down = true;
                        } else if (!mbEvent.isDown) {
                            down = false;
                        }
                        return true;
                    }
                    case InputEventMouseMotion::getUniqueEventId(): {
                        auto &mmEvent = event.toEventType<InputEventMouseMotion>();
                        auto mouseLocal = mouse->localPos;
                        if (down) {
                            auto xpct = 1 - (((double) inputFilter->getRect().width - (double) control->getPos().x) /
                                             (double) inputFilter->getRect().width);
                            auto ypct = 1 - (((double) inputFilter->getRect().height - (double) control->getPos().y) /
                                             (double) inputFilter->getRect().height);
                            if (xpct < 0) xpct = 0;
                            if (xpct > 1) xpct = 1;
                            if (ypct < 0) ypct = 0;
                            if (ypct > 1) ypct = 1;
                            mouseLocal = mouseLocal.clamp(Pos<int>(0, 0), inputFilter->getSize());
                            control->setPosRelative(mouseLocal, offset);
                            hslider->setSliderPct(xpct, true);
                            vslider->setSliderPct(ypct, true);
                            return true;
                        } else {
                            offset = mouseLocal - control->getPos();
                            return true;
                        }
                        break;
                    }
                }
                return false;
            };
            inputFilter->setUnhandledInputCallback(cbInput);

            //connect to slider events
            auto hSlidercb = [&](const Slider::EventSliderValueChanged &event) {
                control->setPos({(int) ((double) inputFilter->getWidth() * event.pct), control->getPos().y});
            };
            inputFilter->subscribe<Slider::EventSliderValueChanged>(hslider, hSlidercb);

            auto vSlidercb = [&](const Slider::EventSliderValueChanged &event) {
                control->setPos({control->getPos().x, (int) ((double) inputFilter->getHeight() * event.pct)});
            };
            inputFilter->subscribe<Slider::EventSliderValueChanged>(vslider, vSlidercb);

        } else if (args.getArg("--saveLoadSceneTest")) {
        } else if (args.getArg("--layoutTestSuperBasic")) {
           Logger::debug() << "Layout test super basic!" << endl;
           auto mainVLayout = HLayout::build("MainLayout");
           mainVLayout->setAnchoring(BaseWidget::Anchor::FILL);
           root->addChild(mainVLayout);
           auto control1 = Control::build("Control1");
           auto control2 = Control::build("Control2");
           control1->getTheme()->background = Style::Fill::SOLID;
           control1->getTheme()->background.colorPrimary.set(ReyEngine::Colors::red);
           control2->getTheme()->background = Style::Fill::SOLID;
           control2->getTheme()->background.colorPrimary.set(ReyEngine::Colors::green);
           mainVLayout->addChild(control1);
           Logger::debug() << "Layout " << mainVLayout->getName() << " child count = " << mainVLayout->getChildren().size() << endl;
           mainVLayout->addChild(control2);
        } else if (args.getArg("--layoutTestBasic")) {
            window.setSize({800, 600});
            Logger::debug() << "Layout test basic!" << endl;
            auto mainVLayout = VLayout::build("MainVLayout");
            mainVLayout->setAnchoring(BaseWidget::Anchor::FILL);
            root->addChild(mainVLayout);

            auto control1 = Control::build("Control1");
            auto control2 = Control::build("Control2");
            auto layout3 = HLayout::build("Layout3");
            control1->getTheme()->background = Style::Fill::SOLID;
            control1->getTheme()->background.colorPrimary.set(ReyEngine::Colors::red);
            control2->getTheme()->background = Style::Fill::SOLID;
            control2->getTheme()->background.colorPrimary.set(ReyEngine::Colors::green);
            mainVLayout->addChild(control1);
            Logger::debug() << "Layout " << mainVLayout->getName() << " child count = " << mainVLayout->getChildren().size() << endl;
            mainVLayout->addChild(control2);
            mainVLayout->addChild(layout3);

           bool passed = true;
            //split layout 3
           static constexpr int SUBCONS = 4;
            for (int i = 0; i < SUBCONS; i++) {
                auto subcontrol = Control::build("SubControl" + to_string(i));
                subcontrol->getTheme()->background = Style::Fill::SOLID;
                subcontrol->getTheme()->background.colorPrimary.set(ReyEngine::Colors::randColor());
                layout3->addChild(subcontrol);
            }

           //assert sizes
           auto assertRect = [&](const std::shared_ptr<BaseWidget>& widget, const Rect<int>& assertRect){
              auto widgetRect = widget->getRect();
              Logger::info() << "Widget " << widget->getName() << " has rect " << widgetRect;
              auto match = widgetRect == assertRect;
              if (match){
                 Logger::info() << ", which is correct" << endl;
              } else {
                 Logger::info() << ", which is incorrect; it should be " << assertRect << endl;
              }
              return match;
           };
            for (int i=0;i<SUBCONS;i++){
               passed &= assertRect(layout3->getChildren().at(i), {200 * i, 0, 200, 200});
            }

           passed &= assertRect(mainVLayout, {0,0,800,600});
           passed &= assertRect(control1, {0, 0, 800, 200});
           passed &= assertRect(control2, {0, 200, 800, 200});
           passed &= assertRect(layout3, {0, 400, 800, 200});
           if (passed){
              Logger::info() << "All is well with this layout" << endl;
           } else {
              Logger::error() << "Layout is incorrect" << endl;
           }
        } else if (args.getArg("--layoutTestConstrained")) {
           Logger::debug() << "Layout test constrained!" << endl;
           auto mainLayout = HLayout::build("MainLayout");
           mainLayout->setAnchoring(BaseWidget::Anchor::FILL);
           root->addChild(mainLayout);
           auto left = Control::build("Left");
           auto center = Control::build("Center");
           auto right = Control::build("Right");
           left->getTheme()->background = Style::Fill::SOLID;
           left->getTheme()->background.colorPrimary.set(ReyEngine::Colors::red);
           center->getTheme()->background = Style::Fill::SOLID;
           center->getTheme()->background.colorPrimary.set(ReyEngine::Colors::green);
           right->getTheme()->background = Style::Fill::SOLID;
           right->getTheme()->background.colorPrimary.set(ReyEngine::Colors::blue);
           static constexpr int CONSTRAINT = 100;
           left->setMaxWidth(CONSTRAINT);
           right->setMaxWidth(CONSTRAINT);
           left->setMinWidth(CONSTRAINT);
           right->setMinWidth(CONSTRAINT);
           mainLayout->addChild(left);
           mainLayout->addChild(center);
           mainLayout->addChild(right);
        } else if (args.getArg("--layoutTest")) {
            Logger::debug() << "Layout test!" << endl;
            auto mainVLayout = VLayout::build("MainVLayout");
            mainVLayout->setAnchoring(BaseWidget::Anchor::FILL);
            mainVLayout->layoutRatios = {1, 5};
            root->addChild(mainVLayout);
            bool flipFlop = false;
            std::shared_ptr<Layout> prevlayout = mainVLayout;
            for (int i = 0; i < 20; i++) {
                std::shared_ptr<Layout> newlayout1;
                std::shared_ptr<Layout> newlayout2;
                if (flipFlop) {
                    newlayout1 = VLayout::build("Vlayout1_" + to_string(i));
                    newlayout2 = VLayout::build("Vlayout2_" + to_string(i));
                } else {
                    newlayout1 = HLayout::build("Hlayout1_" + to_string(i));
                    newlayout2 = HLayout::build("Hlayout2_" + to_string(i));
                }
                newlayout1->layoutRatios = {1, 5};
                newlayout2->layoutRatios = {1, 5};
                flipFlop = !flipFlop;
                auto control = Control::build("control" + to_string(i));
                control->getTheme()->background.set(Style::Fill::SOLID);
                control->getTheme()->background.colorPrimary.set(ColorRGBA::random(254));
                control->setAnchoring(ReyEngine::BaseWidget::Anchor::FILL);
                prevlayout->addChild(newlayout1);
                prevlayout->addChild(newlayout2);
                newlayout1->addChild(control);
                prevlayout = newlayout2;
            }
        } else if (args.getArg("--panelTest")) {
            //add a panel to the layout
            auto panel = Panel::build("Panel");
            panel->setRect({20, 20, 500, 500});
            //add a slider so we can adjust the roundness dynamically
            auto slider = make_shared<Slider>("Slider", Slider::SliderType::HORIZONTAL);
            slider->setRect(Rect<int>(40, 600, 100, 20));
            root->addChild(slider);
            //add an hlayout to fill the slider
            auto sliderHLayout = HLayout::build("sliderLayout");
            sliderHLayout->setRect(slider->getRect().toSizeRect());
            slider->addChild(sliderHLayout);

            //add a name label to the slider
            auto fieldLabel = Label::build("FieldNameRoundness");
            fieldLabel->setText("Roundness: ");
            //set alpha channel transparency so we can see the slider handle
            fieldLabel->getTheme()->background.colorPrimary.value.a = 127;
            //add a value label to the slider
            auto valueLabel = Label::build("roundnessLabel");
            valueLabel->setText(panel->getTheme()->roundness, 3);
            valueLabel->setTheme(fieldLabel->getTheme());

            sliderHLayout->addChild(fieldLabel);
            sliderHLayout->addChild(valueLabel);
            sliderHLayout->setWidth(fieldLabel->getWidth() + valueLabel->getWidth());

            //set the layout ratio so everything can be displayed nicely
            sliderHLayout->layoutRatios = {1.5};

            //resize slider so it fits all our text
            slider->setWidth(sliderHLayout->getWidth());

            auto updateLabel = [panel, valueLabel](const Slider::EventSliderValueChanged &event) {
                double newRound = (float) event.value / 100.0;
                valueLabel->setText(newRound, 3);
                panel->getTheme()->roundness.set(newRound);
            };
            valueLabel->subscribe<Slider::EventSliderValueChanged>(slider, updateLabel);
            root->addChild(panel);

            //add a button to toggle the panel visibility
            auto btnShowPanel = PushButton::build("btnShowPanel");
            auto showPanel = [&](const PushButton::ButtonPressEvent &event) { panel->setVisible(!panel->getVisible()); };
            panel->subscribe<PushButton::ButtonPressEvent>(btnShowPanel, showPanel);
            btnShowPanel->setPos(800, 100);
            root->addChild(btnShowPanel);

            //display some stuff in the panel
            auto panelLabel = Label::build("panelLabel");
            panelLabel->setText("This label is a child of the panel");
            panel->addChildToPanel(panelLabel);
            panel->setResizable(true);

            auto somebutton = PushButton::build("someButton");
            panel->addChildToPanel(somebutton);
            somebutton->setRect({300, 300, 100, 40});

            auto internalPanel = Panel::build("InternalPanel");
            panel->addChildToPanel(internalPanel);
            internalPanel->setResizable(true);
            internalPanel->setRect({5, 50, 100, 100});

            auto internalPanelLabel = Label::build("InternalPanelLabel");
            internalPanel->addChildToPanel(internalPanelLabel);

            auto internalMoveCB = [](Control &internalpanel) {};
            //create another panel that is an extension

            auto internalSuperPanel = SuperPanel::build("superPanel");
            panel->addChildToPanel(internalSuperPanel);
            internalSuperPanel->setRect({30, 30, 100, 100});
        } else if (args.getArg("--editor")) {
            root->addChild(Editor::build("Editor"));
        } else if (args.getArg("--treeTest")) {
            auto treeRoot = std::make_shared<TreeItem>("Root");
            auto tree = Tree::build("Tree");
            tree->setAnchoring(BaseWidget::Anchor::FILL);
            tree->setRoot(treeRoot);
            root->addChild(tree);

            for (auto item: {
                    std::make_shared<TreeItem>("Child1"),
                    std::make_shared<TreeItem>("Child2"),
                    std::make_shared<TreeItem>("Child3")
            }) {
                treeRoot->push_back(item);
            }
            for (auto item: {
                    std::make_shared<TreeItem>("GrandChild01"),
                    std::make_shared<TreeItem>("GrandChild02"),
                    std::make_shared<TreeItem>("GrandChild03")
            }) {
                treeRoot->getChildren()[0]->push_back(item);
            }

            for (auto item: {
                    std::make_shared<TreeItem>("GreatGrandChild01"),
                    std::make_shared<TreeItem>("GreatGrandChild02"),
                    std::make_shared<TreeItem>("GreatGrandChild03")
            }) {
                treeRoot->getChildren()[0]->getChildren()[0]->push_back(item);
            }

            for (auto item: {
                    std::make_shared<TreeItem>("GreatGreatGrandChild01"),
                    std::make_shared<TreeItem>("GreatGreatGrandChild02"),
                    std::make_shared<TreeItem>("GreatGreatGrandChild03")
            }) {
                treeRoot->getChildren()[0]->getChildren()[0]->getChildren()[0]->push_back(item);
            }


            for (auto item: {
                    std::make_shared<TreeItem>("GrandChild11"),
                    std::make_shared<TreeItem>("GrandChild12"),
                    std::make_shared<TreeItem>("GrandChild13")
            }) {
                treeRoot->getChildren()[1]->push_back(item);
            }

            for (auto item: {
                    std::make_shared<TreeItem>("GrandChild21"),
                    std::make_shared<TreeItem>("GrandChild22"),
                    std::make_shared<TreeItem>("GrandChild23")
            }) {
                treeRoot->getChildren()[2]->push_back(item);
            }

            //iterate over the tree
            for (const auto &item: *tree) {
                Logger::debug() << item.getText() << endl;
            }

            tree->setHideRoot(true);
        } else if (args.getArg("--inputPositionTest")) {

        } else if (args.getArg("--spriteTest")) {
            //make sure you've installed any sprite resources in the CMake file or they will not be visible to the engine.
            // The executable runs out of the build directory and not the src directory.
            // use install_file() in the cmake
            auto spriteSheet = Sprite::build("SpriteSheet"); //either pass in a rect or do fit texture later
            spriteSheet->setTexture("test\\characters.png"); //if no rect passed in, region = texture size
            spriteSheet->fitTexture();
            root->addChild(spriteSheet);

            auto animatedSprite = Sprite::build("AnimatedSpriteSheet");
            animatedSprite->setRect(Rect<int>(550, 0, 0, 0));
            animatedSprite->setTexture("test\\characters.png");
            animatedSprite->fitTexture();
            animatedSprite->scale(Vec2<float>(5, 5));
            root->addChild(animatedSprite);
        } else if (args.getArg("--tileMapTest")) {
            //load destination tilemap
            auto tileMap = TileMap::build("destMap");
            FileSystem::File spriteSheet = "test/spritesheet.png";
            auto layerOpt = tileMap->addLayer(spriteSheet);
            if (layerOpt) {
                Logger::debug() << "Tilemap added layer " << layerOpt.value() << " using sprite sheet "
                                << tileMap->getLayer(layerOpt.value()).getAtlas().getFile().abs() << endl;
            } else {
                Logger::error() << "Tilemap " << spriteSheet.abs() << " not found" << endl;
                return 1;
            }
            layerOpt = tileMap->addLayer(spriteSheet);
            if (layerOpt) {
                Logger::debug() << "Tilemap added layer " << layerOpt.value() << " using sprite sheet "
                                << tileMap->getLayer(layerOpt.value()).getAtlas().getFile().abs() << endl;
            } else {
                Logger::error() << "Tilemap " << spriteSheet.abs() << " not found" << endl;
                return 1;
            }

            auto &cursorLayer = tileMap->getLayer(1);
            auto &paintLayer = tileMap->getLayer(0);
            //set atlas tile size (src tile size)
            cursorLayer.getAtlas().setTileSize({16, 16});
            paintLayer.getAtlas().setTileSize({16, 16});

            //set tilemap tile size (dest tile size)
            auto squareEdge = 32;
            Size<R_FLOAT> gridSize = {squareEdge, squareEdge};
            tileMap->setTileSize(gridSize);

            root->addChild(tileMap);
            tileMap->setRect({5, 5, gridSize.x * 20, gridSize.y * 20});

            //source texture
            auto srcTexRect = TextureRect::build("sourceTexRect");
            srcTexRect->setRect({tileMap->getPos().x + tileMap->getWidth() + 30, 5, 100, 100});
            srcTexRect->setTexture(spriteSheet);
            srcTexRect->fitTexture();
            //input forwarderers
            auto srcInputFwd = Control::build("srcInputFwd");
            auto dstInputFwd = Control::build("dstInputFwd");
            srcTexRect->addChild(srcInputFwd);
            srcInputFwd->setAnchoring(BaseWidget::Anchor::FILL);
            tileMap->addChild(dstInputFwd);
            dstInputFwd->setAnchoring(BaseWidget::Anchor::FILL);
            dstInputFwd->getTheme()->background = Style::Fill::NONE;

            auto cbDstRender = [&](const Control &thiz) {
                thiz.drawRectangleLines(dstInputFwd->getRect().toSizeRect(), 2.0, Colors::green);
            };
            dstInputFwd->setRenderCallback(cbDstRender);

            //set tilemap tile size (dest tile size)
            root->addChild(srcTexRect);

            //some globals
            Rect<R_FLOAT> hoverRect;
            Rect<R_FLOAT> selectRect;
            static constexpr int SRC_TILE_SIZE = 16;
            TileMap::TileIndex selectedIndex = -1;

            //source texture input callback
            auto cbSrcInput = [&](const Control &, const InputEvent &event,
                                  const std::optional<UnhandledMouseInput> &mouse) -> bool {
                if (mouse && mouse->isInside) {
                    auto subrect = srcTexRect->getRect().getSubRectAtPos({SRC_TILE_SIZE, SRC_TILE_SIZE}, mouse->localPos);
                    switch (event.eventId) {
                        case InputEventMouseMotion::getUniqueEventId(): {
                            hoverRect = subrect;
                            break;
                        }
                        case InputEventMouseButton::getUniqueEventId():
                            auto mbEvent = event.toEventType<InputEventMouseButton>();
                            if (!mbEvent.isDown) {
                                selectRect = subrect;
                                auto optIndex = cursorLayer.getAtlas().getTileIndex(selectRect.pos());
                                if (optIndex) {
                                    selectedIndex = optIndex.value();
                                }
                                return true;
                            }
                            break;
                    }
                }
                return false;
            };
            srcInputFwd->setUnhandledInputCallback(cbSrcInput);
            srcInputFwd->setMouseExitCallback([&](const Control &) { hoverRect.clear(); });

            //make a simple control that can draw stuff on our texture rect
            auto renderCB = [&](const Control &) {
                if (hoverRect) {
                    ReyEngine::drawRectangleLines(hoverRect + srcTexRect->getGlobalRect().pos(), 2.0, Colors::yellow);
                }
                if (selectRect) {
                    ReyEngine::drawRectangleLines(selectRect + srcTexRect->getGlobalRect().pos(), 2.0, Colors::red);
                }
            };
            srcInputFwd->setRenderCallback(renderCB);

            //add a callback to paint the tile into the tilemap
            TileMap::TileCoord cursorTile(-1, -1);
            auto cbTileMapHover = [&](const TileMap::EventTileMapCellHovered &event) {
                if (selectedIndex != -1) {
                    if (InputManager::isMouseButtonDown(InputInterface::MouseButton::LEFT)) {
                        //paint
                        paintLayer.setTileIndex(event.cellCoord, selectedIndex);
                    } else {
                        //erase old tile
                        cursorLayer.removeTileIndex(cursorTile);
                        //paint the tile temporarily
                        cursorLayer.setTileIndex(event.cellCoord, selectedIndex);
                        cursorTile = event.cellCoord;
                    }
                }
            };
            auto cbTileMapClick = [&](const TileMap::EventTileMapCellClicked &event) {
                if (selectedIndex != -1) {
                    //paint the tile on the paint layer
                    paintLayer.setTileIndex(event.cellCoord, selectedIndex);
                }
            };
            dstInputFwd->subscribe<TileMap::EventTileMapCellHovered>(tileMap, cbTileMapHover);
            dstInputFwd->subscribe<TileMap::EventTileMapCellClicked>(tileMap, cbTileMapClick);
            //delete cursor tile
            auto cbDstMouseExit = [&](const Control &) {
                cursorLayer.removeTileIndex(cursorTile);
            };
            dstInputFwd->setMouseExitCallback(cbDstMouseExit);
        } else if (args.getArg("--buttonTest")) {
            auto label = Label::build("Label");
            auto explainerLabel = Label::build("Explainer");
            explainerLabel->setText(
                    "This will turn green when the button is PRESSED (as opposed to when it is toggled up or down)");
            label->setRect(Rect<int>(50, 650, 1000, 50));
            root->addChild(label);
            auto vlayout = VLayout::build("VLayout");
            vlayout->setRect({100, 100, 200, 300});
            root->addChild(vlayout);

            //callback lambda(s)
            auto cb = [&](const BaseButton::ButtonToggleEvent &event) {
                auto senderName = event.publisher->toPublisherType<BaseWidget>()->getName();
                label->setText(senderName + " : " + (event.down ? "down" : "up"));
                //you can do whatever else here
            };

            auto cbSecret = [&](const BaseButton::ButtonToggleEvent &event) {
                auto senderName = event.publisher->toPublisherType<BaseWidget>()->getName();
                label->setText(senderName + " is the secret button! You win it all!");
                //you can do whatever else here
            };

            auto cbPress = [&](const BaseButton::ButtonPressEvent &event) {
                auto pushButton = event.publisher->toPublisherType<BaseWidget>()->toType<PushButton>();
                //get the control property
                auto &control = pushButton->getProperty<BaseWidget::WidgetProperty>("control");
                auto &pressCount = control.value->getProperty<IntProperty>("pressCount");
                control.value->getTheme()->background.colorPrimary = pressCount % 2 == 0 ? Colors::green : Colors::red;
                pressCount++;
            };

            for (int i = 0; i < 5; i++) {
                static constexpr int secret = 2;
                string istr = to_string(i);
                auto hlayout = HLayout::build("hlayout" + istr);
                auto button = PushButton::build("PushButton" + istr);
                auto control = Control::build("Control" + istr);
                //add the control as a property of the button
                auto widgetProperty = make_shared<BaseWidget::WidgetProperty>("control", control);
                auto pressCount = make_shared<IntProperty>("pressCount", 0);
                control->moveProperty(pressCount);
                button->moveProperty(widgetProperty);
                button->subscribe<PushButton::ButtonPressEvent>(button, cbPress);
                hlayout->layoutRatios.value = {0.85, 0.15};
                hlayout->addChild(button);
                hlayout->addChild(control);
                vlayout->addChild(hlayout);
                //Subscribers don't have to have any relationship with the callbacks they call. In this example,
                // the vlayout is subscribing to the buttons' pushbutton events. However, the callback has nothing
                // to do with the vlayout itself. It's just a lambda. That's just a cool feature of this engine!
                //Callbacks are also typed - every event emits an event, and any handlers that wish to intercept it
                // must have the same signature.
                if (button->getName() == "PushButton" + to_string(secret)) {
                    vlayout->subscribe<BaseButton::ButtonToggleEvent>(button, cbSecret);
                } else {
                    vlayout->subscribe<BaseButton::ButtonToggleEvent>(button, cb);
                }
            }

            auto cbExit = [&](const BaseButton::ButtonToggleEvent &event) {
                //quit
                if (event.down) {
                    Application::exit(Application::ExitReason::CLEAN);
                }
            };

            auto exitButton = PushButton::build("Exit");
            vlayout->addChild(exitButton);
            exitButton->subscribe<BaseButton::ButtonToggleEvent>(exitButton, cbExit);

        } else if (args.getArg("--anchorTest")) {
            auto vLayout = VLayout::build("VLayout");
            vLayout->setAnchoring(BaseWidget::Anchor::FILL);

            //create the 4 Controls
            auto controlTop = Control::build("TopControl");
            auto controlMiddle = Control::build("MiddleControl");
            auto controlBottom = Control::build("BottomControl");
            auto controlFill = Control::build("FillControl");

            vLayout->addChild(controlTop);
            vLayout->addChild(controlMiddle);
            vLayout->addChild(controlBottom);
            vLayout->addChild(controlFill);

            for (auto &control: {controlTop, controlMiddle, controlMiddle, controlFill}) {
                control->getTheme()->background.set(Style::Fill::SOLID);
                control->getTheme()->background.colorPrimary.set(Colors::randColor());
            }

            //create test labels for all anchoring
            auto noneLabel = Label::build("NoneLabel");
            noneLabel->setText("None");
            noneLabel->setAnchoring(BaseWidget::Anchor::NONE);
            controlTop->addChild(noneLabel);

            auto leftLabel = Label::build("LeftLabel");
            leftLabel->setText("Left");
            leftLabel->setAnchoring(BaseWidget::Anchor::LEFT);
            controlTop->addChild(leftLabel);

            auto rightLabel = Label::build("RightLabel");
            rightLabel->setText("Right");
            rightLabel->setAnchoring(BaseWidget::Anchor::RIGHT);
            controlTop->addChild(rightLabel);

            auto topLabel = Label::build("TopLabel");
            topLabel->setText("Top");
            topLabel->setAnchoring(BaseWidget::Anchor::TOP);
            controlTop->addChild(topLabel);

            auto bottomLabel = Label::build("BottomLabel");
            bottomLabel->setText("Bottom");
            bottomLabel->setAnchoring(BaseWidget::Anchor::BOTTOM);
            controlMiddle->addChild(bottomLabel);

            auto bottomLeftLabel = Label::build("BottomLeftLabel");
            bottomLeftLabel->setText("Bottom Left");
            bottomLeftLabel->setAnchoring(BaseWidget::Anchor::BOTTOM_LEFT);
            controlMiddle->addChild(bottomLeftLabel);

            auto bottomRightLabel = Label::build("BottomRightLabel");
            bottomRightLabel->setText("Bottom Right");
            bottomRightLabel->setAnchoring(BaseWidget::Anchor::BOTTOM_RIGHT);
            controlMiddle->addChild(bottomRightLabel);

            auto topLeftLabel = Label::build("TopLeftLabel");
            topLeftLabel->setText("Top Left");
            topLeftLabel->setAnchoring(BaseWidget::Anchor::TOP_LEFT);
            controlMiddle->addChild(topLeftLabel);

            auto topRightLabel = Label::build("TopRightLabel");
            topRightLabel->setText("Top Right");
            topRightLabel->setAnchoring(BaseWidget::Anchor::TOP_RIGHT);
            controlBottom->addChild(topLabel);

            auto fillButton = PushButton::build("FillLabel");
            fillButton->setText("Fill");
            fillButton->setAnchoring(BaseWidget::Anchor::FILL);
            controlFill->addChild(fillButton);

            auto centerLabel = Label::build("CenterLabel");
            centerLabel->setText("Center");
            centerLabel->setAnchoring(BaseWidget::Anchor::CENTER);
            controlBottom->addChild(centerLabel);

            for (auto &label: {noneLabel, leftLabel, rightLabel, topLabel, bottomLabel, bottomLeftLabel, bottomRightLabel,
                               topLeftLabel, topRightLabel, centerLabel}) {
                label->getTheme().get()->background.set(Style::Fill::SOLID);
                label->getTheme().get()->background.colorPrimary.set(Colors::green);
            }

            root->addChild(vLayout);
        } else if (args.getArg("--marginsTest")) {
            //make a grid of layouts
            auto mainvlayout = VLayout::build("MainVLayout");
            mainvlayout->setAnchoring(BaseWidget::Anchor::FILL);
            root->addChild(mainvlayout);
            static constexpr int ROWS = 10;
            static constexpr int COLUMNS = 10;
            mainvlayout->getTheme()->layoutMargins.setAll(5);
            for (int i = 0; i < ROWS; i++) {
                auto row = HLayout::build("Row" + to_string(i));
                row->getTheme()->layoutMargins.setAll(5);
                mainvlayout->addChild(row);
                //add column to row
                for (int j = 0; j < COLUMNS; j++) {
                    auto column = VLayout::build("Row" + to_string(i) + "Column" + to_string(j));
                    row->addChild(column);
                    //set the margins
                    column->getTheme()->layoutMargins.setAll(i + j);
                    //add a control
                    auto control = Control::build("Control " + to_string(i + j));
                    column->addChild(control);
                    //give it a color
                    control->getTheme()->background = Style::Fill::SOLID;
                    control->getTheme()->background.colorPrimary = Colors::randColor();

                    //add a label
                    auto label = Label::build("Label");
                    auto onEnter = [label, control](const Control &) {
                        label->setVisible(true);
                        label->setText(control->localToGlobal(label->getPos()));
                    };
                    auto onExit = [label, control](const Control &) {
                        label->setVisible(false);
                    };
                    control->setMouseEnterCallback(onEnter);
                    control->setMouseExitCallback(onExit);
                    control->setAcceptsHover(true); //get mouse enter and mouse exit events
                    label->setPos(2, 2);
                    label->setVisible(false);
                    control->addChild(label);
                }
            }
        } else if (args.getArg("--hoverTest")) {
            auto mainVLayout = VLayout::build("VLayout");
            mainVLayout->setAnchoring(BaseWidget::Anchor::FILL);
            root->addChild(mainVLayout);
            for (int i = 0; i < 5; i++) {
                auto control = Control::build("Control" + to_string(i));
                auto onHover = [control](const Control &) {
                    std::cout << control->getName() << " got hover!" << endl;
                    control->setVisible(true);
                };
                auto offHover = [control](const Control &) {
                    control->setVisible(false);
                    std::cout << control->getName() << " got hover!" << endl;
                };
                control->setMouseEnterCallback(onHover);
                control->setMouseExitCallback(offHover);
                control->setVisible(false);
                control->getTheme()->background = Style::Fill::SOLID;
                control->getTheme()->background.colorPrimary = ReyEngine::ColorRGBA::random();
                mainVLayout->addChild(control);
            }
        } else if (args.getArg("--tabContainerTest")) {
            Logger::debug() << "Starting tab container test!" << endl;
            auto mainVLayout = VLayout::build("MainVLayout");
            auto tabContainer = TabContainer::build("TabContainer");
            mainVLayout->setAnchoring(BaseWidget::Anchor::FILL);
            mainVLayout->addChild(tabContainer);
            mainVLayout->getTheme()->layoutMargins.setAll(5);
            root->addChild(mainVLayout);

            //create some widgets to dislpay in the tabcontainer
            static constexpr int TAB_COUNT = 4;
            for (int i = 0; i < TAB_COUNT; i++) {
                auto control = Control::build("Control" + to_string(i));
                auto color = Colors::randColor();
                auto renderCB = [control, color](const Control &) {
                    ReyEngine::drawRectangle(control->getGlobalRect(), color);
                };
                control->setRenderCallback(renderCB);
                tabContainer->addChild(control);
            }

        } else if (args.getArg("--xmlTest")) {
            //parse xml file
            FileSystem::File xmlFile("test/xmltest.xml");
            auto vlayout = VLayout::build("mainvlayout");
            vlayout->setAnchoring(BaseWidget::Anchor::FILL);
            root->addChild(vlayout);

            auto document = XML::Parser::loadFile(xmlFile);
            auto xmlRoot = document->getRoot();
            auto rootLabel = Label::build("rootLabel");
            rootLabel->setText(xmlRoot->name() + " =" + xmlRoot->value());
            rootLabel->setMinSize({999, 30});
            rootLabel->setMaxSize({999, 30});
            vlayout->addChild(rootLabel);

            int i = 0;
            int indentLevel = 0;
            std::function<void(const std::shared_ptr<XML::Element> &)> catElement = [&](
                    const std::shared_ptr<XML::Element> &element) {
                auto label = Label::build(to_string(i++));
                std::string indent(4 * indentLevel, ' ');
                label->setText(indent + "<" + element->name());
                label->setMinSize({999, 30});
                label->setMaxSize({999, 30});
                vlayout->addChild(label);
                std::optional<std::shared_ptr<XML::Attribute>> optAttr = element->firstAttr();
                while (optAttr) {
                    auto attr = optAttr.value();
                    auto name = attr->name();
                    auto value = attr->value();
                    label->appendText(" " + name + "=""" + value + """");
                    optAttr = attr->next();
                }
                label->appendText(">");
                auto value = element->value();
                if (!value.empty()) {
                    label->appendText(value);
                }

                auto optChild = element->firstChild();
                std::shared_ptr<Label> retval;
                if (optChild) {
                    indentLevel++;
                    catElement(optChild.value());
                    indentLevel--;
                }

                if (optChild) {
                    label->appendText("\n");
                }
                label->appendText("</" + element->name() + ">");

                auto sibling = element->nextSibling();
                if (sibling) {
                    catElement(sibling.value());
                }
                return label;
            };
            //rebuild tree
            catElement(xmlRoot);
        } else if (args.getArg("--csvTest")) {
            FileSystem::File csvFile("test/csvTest.csv");
            auto parser = CSVParser(csvFile.open(), true);
            //cat out all the data
            if (auto hdr = parser.getHeader()) {
                stringstream ss;
                for (auto &cell: hdr.value().get()) {
                    ss << cell << ",";
                }
                cout << ss.str() << endl;
            }
            auto &rows = parser.getAllRows();
            int rowCount = 0;
            auto it = parser.begin();
            //compare the two methods of data retrieval to ensure they agree
            for (auto &row: rows) {
                stringstream ss;
                auto _it = (*it).begin();
                for (auto &cell: row) {
                    assert(cell == *_it);
                    ss << cell << ",";
                    _it++;
                }
                cout << ss.str() << endl;
                if (rowCount == 3) {
                    assert(row == parser.getRow(3).value().get());
                }
                ++it;
                rowCount++;
            }
        } else if (args.getArg("--dragTest")) {
            //make something we can draw on
            auto canvasControl = Control::build("Control");
            root->addChild(canvasControl);
            canvasControl->setAnchoring(BaseWidget::Anchor::FILL);
            Rect<int> rect(0, 0, 50, 50);
            ReyEngineFont font;
            bool down = false;
            Pos<int> mousePos;
            Pos<int> offset;

            auto cbDrawCanvas = [&](const Control &) {
                ReyEngine::drawRectangle(canvasControl->getRect(), Colors::lightGray);
                ReyEngine::drawRectangle(rect, down ? Colors::red : Colors::green);

                //rect delta line
                if (down) {
                    ReyEngine::drawLine({mousePos, mousePos - offset}, 1, Colors::blue);
                    ReyEngine::drawText("offset = " + offset.toString(), mousePos + Pos<int>(15, 15), font);
                }
            };

            auto cbInput = [&](const Control &, const InputEvent &event,
                               const std::optional<UnhandledMouseInput> mouse) -> bool {
                switch (event.eventId) {
                    case InputEventMouseButton::getUniqueEventId(): {
                        auto &mbEvent = event.toEventType<InputEventMouseButton>();
                        down = mbEvent.isDown && rect.isInside(mbEvent.globalPos);
                        return true;
                    }
                    case InputEventMouseMotion::getUniqueEventId():
                        mousePos = InputManager::getMousePos();
                        if (down) {
                            rect.setPos(mousePos - offset);
                        } else {
                            offset = mousePos - rect.pos();
                        }

                        return true;
                }
                return false;
            };

            canvasControl->setRenderCallback(cbDrawCanvas);
            canvasControl->setUnhandledInputCallback(cbInput);
        } else if (args.getArg("--comboBoxTest")) {
            Size<int> maxSize = {200, 30};
            auto mainvlayout = VLayout::build("MainVLayout");
            mainvlayout->getTheme()->layoutMargins.setAll(2);
            mainvlayout->setAnchoring(BaseWidget::Anchor::FILL);
            root->addChild(mainvlayout);

            for (int i = 0; i < 20; i++) {
                auto combobox = ComboBox::build("Combobox" + to_string(i));
                combobox->setMaxSize(maxSize);
                mainvlayout->addChild(combobox);

                //create a struct to hold data
                struct ColorData : public ComboBoxData {
                    ColorData(const std::string &name, const ColorRGBA &color) : name(name), color(color) {}

                    std::string name;
                    ColorRGBA color;
                };

                //instantiate some data
                std::vector<std::shared_ptr<ColorData>> colors = {
                        make_shared<ColorData>("red", Colors::red),
                        make_shared<ColorData>("green", Colors::green),
                        make_shared<ColorData>("blue", Colors::blue)
                };

                auto hoverCB = [&](const ComboBox::EventComboBoxItemHovered &event) {
                    auto combobox = event.publisher->toPublisherType<BaseWidget>()->toType<ComboBox>();
                    auto data = static_pointer_cast<ColorData>(event.field.data);
                    combobox->getTheme()->background.colorPrimary = data->color;
                };

                auto selectCB = [&](const ComboBox::EventComboBoxItemSelected &event) {
                    auto combobox = event.publisher->toPublisherType<BaseWidget>()->toType<ComboBox>();
                    auto data = static_pointer_cast<ColorData>(event.field.data);
                    combobox->getTheme()->background.colorPrimary = data->color;
                };

                auto menuOpenCB = [&](const ComboBox::EventComboBoxMenuOpened &event) {
                    /**/
                };

                auto menuCloseCB = [&](const ComboBox::EventComboBoxMenuClosed &event) {
                    auto combobox = event.publisher->toPublisherType<BaseWidget>()->toType<ComboBox>();
                    auto data = static_pointer_cast<ColorData>(combobox->getCurrentField().data);
                    combobox->getTheme()->background.colorPrimary = data->color;
                };

                //pack the comboboxes with the data
                for (auto &color: colors) {
                    combobox->addItem(color->name);
                    combobox->getLastField().data = color;
                    combobox->subscribe<ComboBox::EventComboBoxItemHovered>(combobox, hoverCB);
                    combobox->subscribe<ComboBox::EventComboBoxMenuOpened>(combobox, menuOpenCB);
                    combobox->subscribe<ComboBox::EventComboBoxMenuClosed>(combobox, menuCloseCB);
                    combobox->subscribe<ComboBox::EventComboBoxItemSelected>(combobox, selectCB);
                }
            }
        } else if (args.getArg("--configTest")) {
            Config::loadConfig("test/test.lua");
            auto vlayout = VLayout::build("Layout");
            vlayout->setAnchoring(BaseWidget::Anchor::FILL);
            root->addChild(vlayout);
            for (auto &label: {
                    Label::build(Config::getString("testmsg")),
                    Label::build(Config::getString("testtable", "value")),
                    Label::build("an integer = " + to_string(Config::getInt("an_integer"))),
            }) {
                label->setMaxSize({9999, 30});
                vlayout->addChild(label);
            }
        } else if (args.getArg("--lineEditTest")) {
            static constexpr int ROW_HEIGHT = 30;
            Size<int> maxSize = {200, ROW_HEIGHT};
            auto vlayout = VLayout::build("Layout");
            vlayout->setAnchoring(BaseWidget::Anchor::FILL);
            vlayout->getTheme()->layoutMargins.setAll(2);
            root->addChild(vlayout);

            for (int i = 0; i < 20; i++) {
                auto hlayout = HLayout::build("HLayout" + to_string(i));
                hlayout->getTheme()->layoutMargins.setAll(2);
                hlayout->setMaxSize({9999, ROW_HEIGHT});
                vlayout->addChild(hlayout);
                auto linedit = LineEdit::build("LineEdit" + to_string(i));
                linedit->setDefaultText("DefaultText");
                linedit->setMaxSize(maxSize);
                hlayout->addChild(linedit);

                //make a label to echo the contents of the linedit
                auto label = Label::build("Label" + to_string(i));
                label->clear();
                //create callback
                auto cbTextChanged = [label](const LineEdit::EventLineEditTextChanged &event) {
                    label->setText(event.newText);
                };
                label->subscribe<LineEdit::EventLineEditTextChanged>(linedit, cbTextChanged);
                hlayout->addChild(label);
            }
        } else if (args.getArg("--canvasTest")) {
            auto subCanvas = Canvas::build("subCanvas");
            root->addChild(subCanvas);
            subCanvas->setRect({50, 50, 1000, 1000});

            auto cursor = Control::build("Cursor");
            auto cursorRender = [](const Control &thiz) {
                thiz.drawCircle({thiz.getLocalMousePos(), 5}, Colors::black);
            };
            cursor->setRenderCallback(cursorRender);

            auto inputCB = [](const Control &control, const InputEvent &event,
                              const std::optional<UnhandledMouseInput> &mouse) -> Handled {
                cout << control.getName() << "->" << InputManager::getMousePos() << " : " << mouse->localPos << endl;
                return false;
            };

            for (int i = 0; i < 3; i++) {
                auto control = Control::build("Control" + to_string(i));
                auto enterCB = [control](const Control &) { control->getTheme()->background.colorPrimary = Colors::green; };
                auto exitCB = [control](
                        const Control &) { control->getTheme()->background.colorPrimary = Colors::lightGray; };

                control->setMouseEnterCallback(enterCB);
                control->setMouseExitCallback(exitCB);
                subCanvas->addChild(control);
                control->setRect({(float)(i * 100), (float)(i * 100), 100, 100});
                control->setUnhandledInputCallback(inputCB);
            }
            subCanvas->addChild(cursor);

        } else if (args.getArg("--uniqueValueTest")) {
            Logger::info() << "Generating some unique values...you don't need to do anything" << endl;
            Logger::info() << Application::instance().generateUniqueValue() << endl;
            Logger::info() << Application::instance().generateUniqueValue() << endl;
            Logger::info() << Application::instance().generateUniqueValue() << endl;
            Logger::info() << Application::instance().generateUniqueValue() << endl;
            Logger::info() << Application::instance().generateUniqueValue() << endl;
            Logger::info() << Application::instance().generateUniqueValue() << endl;
        }

       else if (args.getArg("--astarTest")) {
          auto vlayout = VLayout::build("VLayout");
          auto instrLabel = Label::build("instrLabel");
          instrLabel->setText("press G = set goal, S = set start");
          auto modeLabel = Label::build("modeLabel");
          auto labelLayout = VLayout::build("labelLayout");
          root->addChild(vlayout);
          root->addChild(labelLayout);
          labelLayout->addChild(instrLabel);
          labelLayout->addChild(modeLabel);
          vlayout->setAnchoring(BaseWidget::Anchor::FILL);

          auto control = Control::build("Control");
          vlayout->addChild(control);

          static constexpr R_FLOAT CELL_SIZE = 8;
          //create graph
          static PathFinding::AStar2D astar(window.getSize().x / CELL_SIZE + 1, window.getSize().y / CELL_SIZE + 1);
          //add connections
          for (auto& node : astar){
             for (auto& neighbor : astar.getNeighbors(node.getCoords())){
                node.setWeight(Application::generateRandom(1,10));
//                if (neighbor.get().getCoords().x == 5){
//                   node.setWeight(0);
//                } else {
//                   node.setWeight(10);
//                }
                node.addConnection(neighbor, false);
             }
          }

          enum class Mode {PICK_PATH, SET_START, SET_GOAL};
          Mode mode = Mode::PICK_PATH;
          auto setModeString = [&](Mode m){
             switch(mode){
                case Mode::PICK_PATH:
                   modeLabel->setText("PICK_PATH");
                   break;
                case Mode::SET_START:
                   modeLabel->setText("SET_START");
                   break;
                case Mode::SET_GOAL:
                   modeLabel->setText("SET_GOAL");
                   break;
             }
          };
          setModeString(mode);
          std::optional<Rect<int>> hoverCell;
          std::optional<std::reference_wrapper<AStar2D::GraphNode>> start;
          std::optional<std::reference_wrapper<AStar2D::GraphNode>> goal;
          auto renderCB = [&](const Control& ctl){
             auto size = window.getSize();
             if (hoverCell){
                ctl.drawRectangle(hoverCell.value(), Colors::yellow);
             }

             //draw start
             if (start){
                auto rect = ctl.getRect().getSubRectAtCoords(Size<R_FLOAT>(CELL_SIZE, CELL_SIZE), Rect<int>::SubRectCoords(start.value().get().getCoords()));
                ctl.drawRectangle(rect, Colors::blue);
             }

             //draw goal
             if (goal){
                auto rect = ctl.getRect().getSubRectAtCoords(Size<R_FLOAT>(CELL_SIZE, CELL_SIZE), Rect<int>::SubRectCoords(goal.value().get().getCoords()));
                ctl.drawRectangle(rect, Colors::red);
             }

             //draw path
             if (start && goal && goal.value().get().getParent()){
                auto parent = goal.value().get().getParent();
                while (parent){
                   if (parent.value().get().getCoords() != start.value().get().getCoords()) {
                      auto rect = ctl.getRect().getSubRectAtCoords(Size<R_FLOAT>(CELL_SIZE, CELL_SIZE), Rect<int>::SubRectCoords(parent.value().get().getCoords()));
                      ctl.drawRectangle(rect, Colors::green);
                   }
                   parent = parent.value().get().getParent();
                }
             }

             //draw grid lines
             for (int x=0;x<size.x/CELL_SIZE+1;x++) {
                Line<R_FLOAT> l(x * CELL_SIZE, 0, x * CELL_SIZE, size.y);
                ctl.drawLine(l, 1, Colors::black);
             }
             for (int y=0;y<size.y/CELL_SIZE+1;y++) {
                Line<R_FLOAT> l(0, y * CELL_SIZE, size.x, y * CELL_SIZE);
                ctl.drawLine(l, 1, Colors::black);
             }
          };

          auto checkPath = [&](){
            if (start && goal){
               astar.findPath(start.value(), goal.value());
            }
          };

          auto unhandledInputCB = [&](Control& ctl, const InputEvent& event, const std::optional<UnhandledMouseInput>& mouse) -> Handled{
             switch(event.eventId){
                case InputEventKey::getUniqueEventId():
                   const auto& keyEvent = event.toEventType<InputEventKey>();
                   auto c = InputInterface::toChar(keyEvent.key);
                   if(!c) return false;
                   switch (c.value()){
                      default:
                         mode = Mode::PICK_PATH;
                         break;
                      case 'G':
                         mode = Mode::SET_GOAL;
                         break;
                      case 'S':
                         mode = Mode::SET_START;
                         break;
                   }
                  setModeString(mode);

             }
             if (!mouse) return false;
             auto cellCoords = ctl.getRect().getSubRectCoord({CELL_SIZE, CELL_SIZE}, mouse.value().localPos);
             auto cellRect = ctl.getRect().getSubRectAtPos({CELL_SIZE, CELL_SIZE}, mouse.value().localPos);
             switch (event.eventId){
                case InputEventMouseMotion::getUniqueEventId():
                   hoverCell = cellRect;
                   return true;
                case InputEventMouseButton::getUniqueEventId():
                   const auto& mbEvent = event.toEventType<InputEventMouseButton>();
                   if (mbEvent.isDown) return false;
                   switch (mbEvent.button){
                      case InputInterface::MouseButton::LEFT:
                         switch (mode){
                            case Mode::SET_START:
                               start = astar.at(cellCoords.get());
                               checkPath();
                               return true;
                            case Mode::SET_GOAL:
                               goal = astar.at(cellCoords.get());
                               checkPath();
                               return true;
                            case Mode::PICK_PATH:
                               //change the weight of the connection(s)
                               break;
                         }
                      default:
                         break;
                   }

             }
             return false;
          };

          control->setRenderCallback(renderCB);
          control->setUnhandledInputCallback(unhandledInputCB);
       }

        else if (args.getArg("--astarTestAuto")) {
           //test a star stuff without any graphics
           static constexpr int CELL_SIZE = 8;
           //create graph
           static PathFinding::AStar2D astar(100,100);
           //add connections
           for (auto& node : astar){
              for (auto& neighbor : astar.getNeighbors(node.getCoords())){
//                 if (neighbor.get().getCoords().)
//                 neighbor.get().addConnection(node);
              }
           }

//           auto vlayout = VLayout::build("VLayout");
//           auto instrLabel = Label::build("instrLabel");
//           instrLabel->setText("press G = set goal, S = set start");
//           auto modeLabel = Label::build("modeLabel");
//           auto labelLayout = VLayout::build("labelLayout");
//           root->addChild(vlayout);
//           root->addChild(labelLayout);
//           labelLayout->addChild(instrLabel);
//           labelLayout->addChild(modeLabel);
//           vlayout->setAnchoring(BaseWidget::Anchor::FILL);
//
//           auto control = Control::build("Control");
//           vlayout->addChild(control);
//
//           enum class Mode {PICK_PATH, SET_START, SET_GOAL};
//           Mode mode = Mode::PICK_PATH;
//           auto setModeString = [&](Mode m){
//              switch(mode){
//                 case Mode::PICK_PATH:
//                    modeLabel->setText("PICK_PATH");
//                    break;
//                 case Mode::SET_START:
//                    modeLabel->setText("SET_START");
//                    break;
//                 case Mode::SET_GOAL:
//                    modeLabel->setText("SET_GOAL");
//                    break;
//              }
//           };
//           setModeString(mode);
//           std::optional<Rect<int>> hoverCell;
//           std::optional<std::reference_wrapper<AStar::GraphNode>> start;
//           std::optional<std::reference_wrapper<AStar::GraphNode>> goal;
//           auto renderCB = [&](const Control& ctl){
//              auto size = window.getSize();
//              if (hoverCell){
//                 ctl.drawRectangle(hoverCell.value(), Colors::yellow);
//              }
//
//              //draw start
//              if (start){
//                 auto rect = ctl.getRect().getSubRect({CELL_SIZE, CELL_SIZE}, Rect<int>::SubRectCoords(start.value().get().getCoords()));
//                 ctl.drawRectangle(rect, Colors::blue);
//              }
//
//              //draw goal
//              if (goal){
//                 auto rect = ctl.getRect().getSubRect({CELL_SIZE, CELL_SIZE}, Rect<int>::SubRectCoords(goal.value().get().getCoords()));
//                 ctl.drawRectangle(rect, Colors::red);
//              }
//
//              //draw path
//              if (start && goal && goal.value().get().getParent()){
//                 auto parent = goal.value().get().getParent();
//                 while (parent){
//                    if (parent.value().get().getCoords() != start.value().get().getCoords()) {
//                       auto rect = ctl.getRect().getSubRect({CELL_SIZE, CELL_SIZE}, Rect<int>::SubRectCoords(parent.value().get().getCoords()));
//                       ctl.drawRectangle(rect, Colors::green);
//                    }
//                    parent = parent.value().get().getParent();
//                 }
//              }
//
//              //draw grid lines
//              for (int x=0;x<size.x/CELL_SIZE+1;x++) {
//                 Line l(x * CELL_SIZE, 0, x * CELL_SIZE, size.y);
//                 ctl.drawLine(l, 1, Colors::black);
//              }
//              for (int y=0;y<size.y/CELL_SIZE+1;y++) {
//                 Line l(0, y * CELL_SIZE, size.x, y * CELL_SIZE);
//                 ctl.drawLine(l, 1, Colors::black);
//              }
//           };
//
//           auto checkPath = [&](){
//              if (start && goal){
//                 astar.findPath(start.value(), goal.value());
//              }
//           };
//
//           auto unhandledInputCB = [&](Control& ctl, const InputEvent& event, const std::optional<UnhandledMouseInput>& mouse) -> Handled{
//              switch(event.eventId){
//                 case InputEventKey::getUniqueEventId():
//                    const auto& keyEvent = event.toEventType<InputEventKey>();
//                    auto c = InputInterface::toChar(keyEvent.key);
//                    if(!c) return false;
//                    switch (c.value()){
//                       default:
//                          mode = Mode::PICK_PATH;
//                          break;
//                       case 'G':
//                          mode = Mode::SET_GOAL;
//                          break;
//                       case 'S':
//                          mode = Mode::SET_START;
//                          break;
//                    }
//                    setModeString(mode);
//
//              }
//              if (!mouse) return false;
//              auto cellCoords = ctl.getRect().getSubRectCoord({CELL_SIZE, CELL_SIZE}, mouse.value().localPos);
//              auto cellRect = ctl.getRect().getSubRect({CELL_SIZE, CELL_SIZE}, mouse.value().localPos);
//              switch (event.eventId){
//                 case InputEventMouseMotion::getUniqueEventId():
//                    hoverCell = cellRect;
//                    return true;
//                 case InputEventMouseButton::getUniqueEventId():
//                    const auto& mbEvent = event.toEventType<InputEventMouseButton>();
//                    if (mbEvent.isDown) return false;
//                    switch (mbEvent.button){
//                       case InputInterface::MouseButton::LEFT:
//                          switch (mode){
//                             case Mode::SET_START:
//                                start = astar.at(cellCoords.get());
//                                checkPath();
//                                return true;
//                             case Mode::SET_GOAL:
//                                goal = astar.at(cellCoords.get());
//                                checkPath();
//                                return true;
//                             case Mode::PICK_PATH:
//                                //change the weight of the connection(s)
//                                break;
//                          }
//                       default:
//                          break;
//                    }
//
//              }
//              return false;
//           };
//
//           control->setRenderCallback(renderCB);
//           control->setUnhandledInputCallback(unhandledInputCB);

        }

        else if (args.getArg("--camera2dTest")) {
            //create nodes
            auto camera = ReyEngine::Camera2D::build("Camera2D", getScreenSize());
            auto cameraUILabel = Label::build("cameraUI");
            auto background = Control::build("background");
            auto backgroundLabel = Label::build("backgroundLabel");
            auto pushButton = PushButton::build("PushButton");

            //setup background
            background->getTheme()->background.value = Style::Fill::GRADIENT;
            background->getTheme()->background.colorPrimary.value = Colors::blue;
            background->getTheme()->background.colorSecondary.value = Colors::red;
            background->setRect({0, 0, 1000, 1000});

            pushButton->setPos({300,300});

            // add nodes in correct order
            root->addChild(background);
            background->addChild(backgroundLabel);
            background->addChild(camera);
            camera->addChild(cameraUILabel);
            background->addChild(pushButton);

            //controls
            auto wkCamera = std::weak_ptr<ReyEngine::Camera2D>(camera);
            auto cbBGUnhandledInput = [wkCamera, cameraUILabel](Control &, const InputEvent &event,
                                                                const std::optional<UnhandledMouseInput> &mouse) -> Handled {
                if (event.isEvent<InputEventMouseWheel>()) {
                    auto wheelEvent = event.toEventType<InputEventMouseWheel>();
                    double wheel = wheelEvent.wheelMove.y;
                    if (wheel) {
                        auto camera = wkCamera.lock();
                        if (camera) {
                           Logger::info() << "Wheel value = " << wheel << endl;
                            camera->setZoom(camera->getZoom() * (1 + wheel / 10));
                            Logger::info() << "New camera zoom level = " << camera->getZoom() << endl;
                            return true;
                        }
                    }
                }
                return false;
            };

            auto cbBGProcess = [wkCamera, cameraUILabel, backgroundLabel, background](Control &ctl, double dt) {
                //make sure camera still exists
                auto camera = wkCamera.lock();
                if (!camera) return;
                int moveSpeed = 10;
                Vec2<int> mvVec;
                double rotation = 0;
                if (InputManager::isKeyDown(InputInterface::KeyCode::KEY_W)) {
                    mvVec += {0, -1};
                }
                if (InputManager::isKeyDown(InputInterface::KeyCode::KEY_A)) {
                    mvVec += {-1, 0};
                }
                if (InputManager::isKeyDown(InputInterface::KeyCode::KEY_S)) {
                    mvVec += {0, 1};
                }
                if (InputManager::isKeyDown(InputInterface::KeyCode::KEY_D)) {
                    mvVec += {1, 0};
                }
                if (InputManager::isKeyDown(InputInterface::KeyCode::KEY_Q)) {
                    rotation += 1;
                }
                if (InputManager::isKeyDown(InputInterface::KeyCode::KEY_E)) {
                    rotation -= 1;
                }
                if (mvVec) {
                    auto newVec = camera->getPos() + (mvVec * moveSpeed);
                    camera->setPos(newVec);
                }
                if (rotation) {
                    camera->setRotation(camera->getRotation() + rotation);
                }
                cameraUILabel->setText("Label: " + string(cameraUILabel->getPos())
                                       + "\nCamera Pos = " + string(camera->getPos())
                                       + "\nCamera Target = " + string(camera->getTarget())
                                       + "\nCamera Offset = " + string(camera->getOffset())
                                       + "\nWindow Mouse Pos = " + InputManager::getMousePos().toString()
                                       + "\nBckgnd Mouse Pos = " + camera->nearToFar(InputManager::getMousePos()).toString()
                );
                backgroundLabel->setText("BG pos = " + backgroundLabel->getPos().toString()
                                        + "\nLocal Mouse Pos = " + background->globalToLocal(camera->farToNear(InputManager::getMousePos())).toString()
                );

            };
            background->setUnhandledInputCallback(cbBGUnhandledInput);
            background->setProcessCallback(cbBGProcess);
        } else if (args.getArg("--readFileTest")) {
            auto file = FileSystem::File("test/test.scn").open();
            auto fileCopy = file;
            //cat out the contents of the file as a whole
            Logger::info() << "Catting out contents of " << file->file().abs() << " using readfile" << endl;
            stringstream ss;
            for (auto c: file->readFile()) {
                ss << c;
            }
            std::cout << ss.str() << endl;

            //cat out the contents of the file byte by byte
            Logger::info() << "Catting out contents of " << fileCopy->file().abs() << " using getByte" << endl;
            bool done = false;
            ss.clear();
            while (!fileCopy->isEof()) {
                static constexpr int strsize = 10;
                for (auto c: fileCopy->readBytes(strsize)) {
                    ss << c;
                }
            }
            std::cout << ss.str() << endl;
        } else if (args.getArg("--inspector")) {
            //make a vlayout - put a widget at the top and the inspector below it
            root->addChild(VLayout::build("VLayout"));
            auto someWidget = Label::build("SomeWidget");
            auto inspector = Inspector::build("Inspector");
            someWidget->getTheme()->background.set(Style::Fill::SOLID);
            someWidget->getTheme()->background.colorPrimary.set(Colors::blue);
            root->addChild(someWidget);
            root->addChild(inspector);
            inspector->inspect(someWidget);
        } else if (args.getArg("--3dTest")) {
            //declare a viewport
            auto viewport = Viewport::build("viewport");
            root->addChild(viewport);
            viewport->setAnchoring(ReyEngine::BaseWidget::Anchor::FILL);

            auto vlayout = VLayout::build("mainvlayout");
            viewport->BaseWidget::addChild(vlayout);
            vlayout->setAnchoring(ReyEngine::BaseWidget::Anchor::FILL);

            auto label = Label::build("UI");
            viewport->addChild2d(label);

            auto body = ModelBody::build("TestBody");
            auto modelOpt = Model3D::load("test/suzanne.obj");
            if (!modelOpt) throw std::runtime_error("Unable to load suzanne");
            body->setModel(modelOpt.value());
            //map a random texture to it
            auto tex = make_shared<ReyTexture>("test/spritesheet.png");
            body->setTexture(tex);
            viewport->addChild3d(body);
            body->setVisible(true);

            //add some mouse controls
            auto inputCB = [&](const Control &control, const InputEvent &event,
                               const std::optional<UnhandledMouseInput> &mouse) -> Handled {
                if (!mouse) return false;
                auto ray = viewport->castRayClosest(mouse.value().localPos);
                //see if the ray hit anything;
                if (!ray) return false;
                cout << control.getName() << "->" << InputManager::getMousePos() << " : " << mouse->localPos << " = "
                     << ray.value() << endl;
                return false;
            };

            auto mouseOverlay = Control::build("MouseOverlay");
            mouseOverlay->setAnchoring(ReyEngine::BaseWidget::Anchor::FILL);
            mouseOverlay->getTheme()->background.value = Style::Fill::NONE;
            mouseOverlay->setUnhandledInputCallback(inputCB);
            viewport->addChild2d(mouseOverlay);

        } else if (args.getArg("--freeTest")) {
           auto label = Label::build("Label");
           auto btn = PushButton::build("Free!");
           btn->setPos(500,500);
           root->addChild(label);
           root->addChild(btn);

           auto pbCallback = [&](const PushButton::ButtonPressEvent& event){
              Logger::info() << "Removing child label" << endl;
              root->removeChild("Label", true);
           };

           btn->subscribe<PushButton::ButtonPressEvent>(btn, pbCallback);
        } else if (args.getArg("--rotationTest")) {
           auto label0 = Label::build("label0");
           label0->setRotation(22.5);
           label0->setPos(50, 0);
           label0->setText("This label should be located at {50,0} and rotated slightly");
           auto label1 = Label::build("label1");
           label1->setPos(50, 0);
           label1->setRotation(22.5);
           auto label2 = Label::build("label2");
           label2->setPos(50, 0);
           label2->setRotation(22.5);

           root->addChild(label0);
           label0->addChild(label1);
           label1->addChild(label2);
        } else {
            cout << args.getDocString() << endl;
            return 0;
        }

        root->setAnchoring(BaseWidget::Anchor::FILL);
        window.exec();
    }
    return 0;
}