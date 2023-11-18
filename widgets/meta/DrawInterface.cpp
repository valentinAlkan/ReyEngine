#include "DrawInterface.h"
#include "Application.h"
using namespace GFCSDraw;

/////////////////////////////////////////////////////////////////////////////////////////
Pos<double> GFCSDraw::getScreenCenter() {
   return {((float)GetScreenWidth())/2, ((float)GetScreenHeight())/2};
}

/////////////////////////////////////////////////////////////////////////////////////////
Size<int> GFCSDraw::getScreenSize() {
   return {GetScreenWidth(),GetScreenHeight()};
}

/////////////////////////////////////////////////////////////////////////////////////////
void GFCSDraw::drawText(const std::string &text, const GFCSDraw::Pos<int>& pos, const GFCSDrawFont& font) {
//   void DrawTextPro(Font font, const char *text, Vector2 position, Vector2 origin, float rotation, float fontSize, float spacing, Color tint);
   DrawTextPro(font.font, text.c_str(), {(float)pos.x, (float)pos.y}, {0, 0}, 0, font.size, font.spacing, font.color);
}

/////////////////////////////////////////////////////////////////////////////////////////
void GFCSDraw::drawRectangle(const GFCSDraw::Rect<int>& r, GFCSDraw::ColorRGBA color) {
   DrawRectangle(r.x, r.y, r.width, r.height, color);
}

/////////////////////////////////////////////////////////////////////////////////////////
void GFCSDraw::drawRectangleRounded(const GFCSDraw::Rect<float>& r, float roundness, int segments, Color color) {
   DrawRectangleRounded({r.x, r.y, r.width, r.height}, roundness, segments, color);
}


/////////////////////////////////////////////////////////////////////////////////////////
void GFCSDraw::drawRectangleLines(const Rect<float>& r, float lineThick, Color color) {
   DrawRectangleLinesEx({r.x, r.y, r.width, r.height}, lineThick, color);
}

/////////////////////////////////////////////////////////////////////////////////////////
void GFCSDraw::drawRectangleRoundedLines(const GFCSDraw::Rect<float>& r, float roundness, int segments, float lineThick, Color color) {
   DrawRectangleRoundedLines({r.x, r.y, r.width, r.height}, roundness, segments, lineThick, color);
}

/////////////////////////////////////////////////////////////////////////////////////////
void GFCSDraw::drawRectangleGradientV(const GFCSDraw::Rect<int>& rect, Color color1, Color color2) {
   DrawRectangleGradientV(rect.x, rect.y, rect.width, rect.height, color1, color2);
}

/////////////////////////////////////////////////////////////////////////////////////////
void GFCSDraw::drawLine(const Line<int>& line, GFCSDraw::ColorRGBA color) {
   DrawLine(line.a.x, line.a.y, line.b.x, line.b.y, color);
}

/////////////////////////////////////////////////////////////////////////////////////////
void GFCSDraw::drawTextCentered(const std::string& text, const Pos<int>& pos, const GFCSDrawFont& font){
   auto textWidth = MeasureText(text.c_str(), font.size);
   float newX = (float)pos.x - (float)textWidth / 2;
   float newY = (float)pos.y - (float)font.size / 2;
   drawText(text, Vec2<int>((int)newX, (int)newY), font);
}

/////////////////////////////////////////////////////////////////////////////////////////
void GFCSDraw::drawTextRelative(const std::string& text, const Pos<int>& relPos, const GFCSDrawFont& font){
   //draw text relative as a percentage of the screen
   Vector2 screenSize = {(float)GetScreenWidth(), (float)GetScreenHeight()};
   auto newX = screenSize.x * relPos.x / 100.0;
   auto newY = screenSize.y * relPos.y / 100.0;
   drawText(text, {(int)newX, (int)newY}, font);
}

/////////////////////////////////////////////////////////////////////////////////////////
GFCSDraw::RenderTarget::RenderTarget(const Size<int>& size)
: _size(size)
{
   auto doReady = [this]() {
      _tex = LoadRenderTexture(_size.x, _size.y);
      _texLoaded = true;
   };
   Application::registerForApplicationReady(doReady);
}

/////////////////////////////////////////////////////////////////////////////////////////
void GFCSDraw::RenderTarget::resize(const Size<int> &newSize) {
   if (_texLoaded) {
      UnloadRenderTexture(_tex);
      LoadRenderTexture(newSize.x, newSize.y);
   }
   _size = newSize;
}

/////////////////////////////////////////////////////////////////////////////////////////
void GFCSDraw::setWindowSize(GFCSDraw::Size<int> size) {
   SetWindowSize(size.x, size.y);
}

/////////////////////////////////////////////////////////////////////////////////////////
Size<int> GFCSDraw::getWindowSize() {
   return {GetRenderWidth(), GetRenderHeight()};
}

/////////////////////////////////////////////////////////////////////////////////////////
GFCSDraw::Pos<int> GFCSDraw::getWindowPosition() {
   return GetWindowPosition();
}

/////////////////////////////////////////////////////////////////////////////////////////
void GFCSDraw::setWindowPosition(GFCSDraw::Pos<int> pos) {
   SetWindowPosition(pos.x, pos.y);
}

/////////////////////////////////////////////////////////////////////////////////////////
void GFCSDraw::maximizeWindow() {
   //desktop only, only if window resizbale
   MaximizeWindow();
}
/////////////////////////////////////////////////////////////////////////////////////////
void GFCSDraw::minimizeWindow() {
   //desktop only, only if window resizable
   MinimizeWindow();
}

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
GFCSDraw::GFCSDrawFont::GFCSDrawFont(const std::string& fontFile){
   if (fontFile.empty()) {
      font = GetFontDefault();
   } else {
      font = LoadFont(fontFile.c_str());
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
GFCSDrawFont GFCSDraw::getDefaultFont() {
   return GFCSDrawFont();
}

GFCSDrawFont &GFCSDrawFont::operator=(const GFCSDrawFont &rhs)
{
   size = rhs.size;
   spacing = rhs.spacing;
   isDefault = rhs.isDefault;
   fileName = rhs.fileName;
   font = LoadFont(fileName.c_str());
   return *this;
}

/////////////////////////////////////////////////////////////////////////////////////////
Size<int> GFCSDrawFont::measure(const std::string &text) const {
   return GFCSDraw::measureText(text, *this);
}