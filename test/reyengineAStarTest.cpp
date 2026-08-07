#include "Window.h"
#include "ReyEngine.h"
#include "ReyStarServer.h"

using namespace std;
using namespace ReyEngine;
using namespace ReyStar;

class AStarTile{

};

class AStarWidget {

};


int main(int argc, char** argv){
    constexpr auto screenSize = Size{1920,1080} - Size(100,100);
    auto& window = Application::createWindowPrototype("AStarServer", screenSize.x, screenSize.y, {WindowFlags::RESIZE}, 60)->createWindow();
    auto root = window.getCanvas();



    window.exec();
    return 0;
}