#include "DrawInterface.h"
using namespace GFCSDraw;

/////////////////////////////////////////////////////////////////////////////////////////
Vec2<double> getScreenCenter() {
   return {((float)GetScreenWidth())/2, ((float)GetScreenHeight())/2};
}
/////////////////////////////////////////////////////////////////////////////////////////
void drawTextCentered(const std::string& text, const Vec2<int>& pos, int fontSize, Color color){
   auto textWidth = MeasureText(text.c_str(), fontSize);
   float newX = pos.x - (float)textWidth / 2;
   float newY = pos.y - (float)fontSize / 2;
   DrawText(text.c_str(), (int)newX, (int)newY, fontSize, color);
}

/////////////////////////////////////////////////////////////////////////////////////////
void drawTextRelative(const std::string& text, const Vec2<int>& relPos, int fontSize, Color color){
   //draw text relative as a percentage of the screen
   Vector2 screenSize = {(float)GetScreenWidth(), (float)GetScreenHeight()};
   auto newX = screenSize.x * relPos.x / 100.0;
   auto newY = screenSize.y * relPos.y / 100.0;
   DrawText(text.c_str(), (int)newX, (int)newY, fontSize, color);
}

/////////////////////////////////////////////////////////////////////////////////////////
Vec2<int> getMousePos(){
   return GetMousePosition();
}

/////////////////////////////////////////////////////////////////////////////////////////
float getFrameDelta(){
   return GetFrameTime();
}