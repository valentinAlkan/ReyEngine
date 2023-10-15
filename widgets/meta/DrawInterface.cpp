#include "DrawInterface.h"
using namespace GFCSDraw;

/////////////////////////////////////////////////////////////////////////////////////////
Vec2<double> GFCSDraw::getScreenCenter() {
   return {((float)GetScreenWidth())/2, ((float)GetScreenHeight())/2};
}

/////////////////////////////////////////////////////////////////////////////////////////
void GFCSDraw::drawText(const std::string &text, const GFCSDraw::Vec2<int> &pos, int fontSize, Color color) {
   DrawText(text.c_str(), pos.x, pos.y, fontSize, color);
}

/////////////////////////////////////////////////////////////////////////////////////////
void GFCSDraw::drawRectangle(const GFCSDraw::Rect<int>& r, Color color) {
   DrawRectangle(r.x, r.y, r.width, r.height, color);
}

/////////////////////////////////////////////////////////////////////////////////////////
void GFCSDraw::drawRectangleRounded(const GFCSDraw::Rect<float>& r, float roundness, int segments, Color color) {
   DrawRectangleRounded({r.x, r.y, r.width, r.height}, roundness, segments, color);
}


/////////////////////////////////////////////////////////////////////////////////////////
void GFCSDraw::drawRectangleRoundedLines(const GFCSDraw::Rect<float>& r, float roundness, int segments, float lineThick, Color color) {
   DrawRectangleRoundedLines({r.x, r.y, r.width, r.height}, roundness, segments, lineThick, color);
}

/////////////////////////////////////////////////////////////////////////////////////////
void GFCSDraw::drawTextCentered(const std::string& text, const Vec2<int>& pos, int fontSize, Color color){
   auto textWidth = MeasureText(text.c_str(), fontSize);
   float newX = pos.x - (float)textWidth / 2;
   float newY = pos.y - (float)fontSize / 2;
   drawText(text, Vec2<int>(newX, newY), fontSize, color);
}

/////////////////////////////////////////////////////////////////////////////////////////
void GFCSDraw::drawTextRelative(const std::string& text, const Vec2<int>& relPos, int fontSize, Color color){
   //draw text relative as a percentage of the screen
   Vector2 screenSize = {(float)GetScreenWidth(), (float)GetScreenHeight()};
   auto newX = screenSize.x * relPos.x / 100.0;
   auto newY = screenSize.y * relPos.y / 100.0;
   drawText(text, Vec2<int>(newX, newY), fontSize, color);
}