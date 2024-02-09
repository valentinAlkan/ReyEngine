#include "DrawInterface.h"
#include "Application.h"
using namespace ReyEngine;
using namespace std;

/////////////////////////////////////////////////////////////////////////////////////////
ReyEngine::FileSystem::Path::Path(const std::string &path)
: path(path)
{
}

/////////////////////////////////////////////////////////////////////////////////////////
bool ReyEngine::FileSystem::Path::exists() {
 //todo
 return true;
}

/////////////////////////////////////////////////////////////////////////////////////////
std::optional<FileSystem::Path> ReyEngine::FileSystem::Path::head() {
   //todo:
   return nullopt;
}

/////////////////////////////////////////////////////////////////////////////////////////
std::optional<FileSystem::Path> ReyEngine::FileSystem::Path::tail() {
   //todo:
   return nullopt;
}

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
ReyEngine::ReyTexture::ReyTexture(FileSystem::File file) {
   auto doReady = [&]() {
      _tex = LoadTexture(file.str().c_str());
      _texLoaded = true;
   };
   Application::registerForApplicationReady(doReady);
}
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
Pos<double> ReyEngine::getScreenCenter() {
   return {((float)GetScreenWidth())/2, ((float)GetScreenHeight())/2};
}

/////////////////////////////////////////////////////////////////////////////////////////
Size<int> ReyEngine::getScreenSize() {
   return {GetScreenWidth(),GetScreenHeight()};
}

/////////////////////////////////////////////////////////////////////////////////////////
void ReyEngine::drawText(const std::string &text, const ReyEngine::Pos<int>& pos, const ReyEngineFont& font) {
//   void DrawTextPro(Font font, const char *text, Vector2 position, Vector2 origin, float rotation, float fontSize, float spacing, Color tint);
   DrawTextPro(font.font, text.c_str(), {(float)pos.x, (float)pos.y}, {0, 0}, 0, font.size, font.spacing, font.color);
}

/////////////////////////////////////////////////////////////////////////////////////////
void ReyEngine::drawRectangle(const ReyEngine::Rect<int>& r, const ReyEngine::ColorRGBA& color) {
   DrawRectangle(r.x, r.y, r.width, r.height, color);
}

/////////////////////////////////////////////////////////////////////////////////////////
void ReyEngine::drawRectangleRounded(const ReyEngine::Rect<float>& r, float roundness, int segments, const ReyEngine::ColorRGBA& color) {
   DrawRectangleRounded({r.x, r.y, r.width, r.height}, roundness, segments, color);
}


/////////////////////////////////////////////////////////////////////////////////////////
void ReyEngine::drawRectangleLines(const Rect<float>& r, float lineThick, const ReyEngine::ColorRGBA& color) {
   DrawRectangleLinesEx({r.x, r.y, r.width, r.height}, lineThick, color);
}

/////////////////////////////////////////////////////////////////////////////////////////
void ReyEngine::drawRectangleRoundedLines(const ReyEngine::Rect<float>& r, float roundness, int segments, float lineThick, const ReyEngine::ColorRGBA& color) {
   DrawRectangleRoundedLines({r.x, r.y, r.width, r.height}, roundness, segments, lineThick, color);
}

/////////////////////////////////////////////////////////////////////////////////////////
void ReyEngine::drawRectangleGradientV(const ReyEngine::Rect<int>& rect, ReyEngine::ColorRGBA& color1, const ReyEngine::ColorRGBA& color2) {
   DrawRectangleGradientV(rect.x, rect.y, rect.width, rect.height, color1, color2);
}

/////////////////////////////////////////////////////////////////////////////////////////
void ReyEngine::drawLine(const Line<int>& line, const ReyEngine::ColorRGBA& color) {
   DrawLine(line.a.x, line.a.y, line.b.x, line.b.y, color);
}

/////////////////////////////////////////////////////////////////////////////////////////
void ReyEngine::drawTexture(ReyTexture texture, const Rect<int> &source, const Rect<int> &dest, float rotation, float scale, const ReyEngine::ColorRGBA &tint){
   auto tex = texture.getTexture();
   Rectangle rSource =  {(float)source.x, (float)source.y, (float)source.width, (float)source.height};
   Rectangle rDest =  {(float)dest.x, (float)dest.y, (float)dest.width, (float)dest.height};
   DrawTexturePro(tex, rSource, rDest, {0,0},  rotation, tint);
}

/////////////////////////////////////////////////////////////////////////////////////////
void ReyEngine::drawTextCentered(const std::string& text, const Pos<int>& pos, const ReyEngineFont& font){
   auto textWidth = MeasureText(text.c_str(), font.size);
   float newX = (float)pos.x - (float)textWidth / 2;
   float newY = (float)pos.y - (float)font.size / 2;
   drawText(text, Vec2<int>((int)newX, (int)newY), font);
}

/////////////////////////////////////////////////////////////////////////////////////////
void ReyEngine::drawTextRelative(const std::string& text, const Pos<int>& relPos, const ReyEngineFont& font){
   //draw text relative as a percentage of the screen
   Vector2 screenSize = {(float)GetScreenWidth(), (float)GetScreenHeight()};
   auto newX = screenSize.x * relPos.x / 100.0;
   auto newY = screenSize.y * relPos.y / 100.0;
   drawText(text, {(int)newX, (int)newY}, font);
}

/////////////////////////////////////////////////////////////////////////////////////////
ReyEngine::RenderTarget::RenderTarget(const Size<int>& size)
: _size(size)
{
   auto doReady = [this]() {
      _tex = LoadRenderTexture(_size.x, _size.y);
      _texLoaded = true;
   };
   Application::registerForApplicationReady(doReady);
}

/////////////////////////////////////////////////////////////////////////////////////////
void ReyEngine::RenderTarget::resize(const Size<int> &newSize) {
   if (_texLoaded) {
      UnloadRenderTexture(_tex);
      LoadRenderTexture(newSize.x, newSize.y);
   }
   _size = newSize;
}

/////////////////////////////////////////////////////////////////////////////////////////
void ReyEngine::setWindowSize(ReyEngine::Size<int> size) {
   SetWindowSize(size.x, size.y);
}

/////////////////////////////////////////////////////////////////////////////////////////
Size<int> ReyEngine::getWindowSize() {
   return {GetRenderWidth(), GetRenderHeight()};
}

/////////////////////////////////////////////////////////////////////////////////////////
ReyEngine::Pos<int> ReyEngine::getWindowPosition() {
   return GetWindowPosition();
}

/////////////////////////////////////////////////////////////////////////////////////////
void ReyEngine::setWindowPosition(ReyEngine::Pos<int> pos) {
   SetWindowPosition(pos.x, pos.y);
}

/////////////////////////////////////////////////////////////////////////////////////////
void ReyEngine::maximizeWindow() {
   //desktop only, only if window resizbale
   MaximizeWindow();
}
/////////////////////////////////////////////////////////////////////////////////////////
void ReyEngine::minimizeWindow() {
   //desktop only, only if window resizable
   MinimizeWindow();
}

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
ReyEngine::ReyEngineFont::ReyEngineFont(const std::string& fontFile){
   if (fontFile.empty()) {
      font = GetFontDefault();
   } else {
      font = LoadFont(fontFile.c_str());
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
ReyEngineFont ReyEngine::getDefaultFont() {
   return ReyEngineFont();
}

ReyEngineFont &ReyEngineFont::operator=(const ReyEngineFont &rhs)
{
   size = rhs.size;
   spacing = rhs.spacing;
   isDefault = rhs.isDefault;
   fileName = rhs.fileName;
   font = LoadFont(fileName.c_str());
   return *this;
}

/////////////////////////////////////////////////////////////////////////////////////////
Size<int> ReyEngineFont::measure(const std::string &text) const {
   return ReyEngine::measureText(text, *this);
}