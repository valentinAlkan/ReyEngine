#include "DrawInterface.h"
#include "Application.h"
#include "rlgl.h"

using namespace ReyEngine;
using namespace std;

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
ReyTexture::ReyTexture(const FileSystem::File& file) {
   loadTexture(file);
   _path = file.str();
}

/////////////////////////////////////////////////////////////////////////////////////////
void ReyTexture::loadTexture(const FileSystem::File &file) {
   auto doReady = [&]() {
      auto path = file.abs();
      if (!file.exists()){
         Logger::warn() << "LoadTexture failure: Texture file " << file.abs() << " does not exist!" << endl;
      }
      _tex = LoadTexture(path.c_str());
      _texLoaded = true;
      _path = file.str();
      size = {_tex.width, _tex.height};
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
void ReyEngine::drawRectangleGradientV(const ReyEngine::Rect<int>& rect, const ReyEngine::ColorRGBA& color1, const ReyEngine::ColorRGBA& color2) {
   DrawRectangleGradientV(rect.x, rect.y, rect.width, rect.height, color1, color2);
}

/////////////////////////////////////////////////////////////////////////////////////////
void ReyEngine::drawCircle(const Circle& circle, const ReyEngine::ColorRGBA &color) {
   DrawCircle(circle.center.x, circle.center.y, circle.radius, color);
}

/////////////////////////////////////////////////////////////////////////////////////////
void ReyEngine::drawCircleSector(const CircleSector& sector, const ReyEngine::ColorRGBA &color, int segments) {
   DrawCircleSector(sector.center, sector.radius, sector.startAngle, sector.endAngle, segments, color);
}

/////////////////////////////////////////////////////////////////////////////////////////
void ReyEngine::drawCircleSectorLines(const CircleSector& sector, const ReyEngine::ColorRGBA &color, int segments) {
   DrawCircleSectorLines(sector.center, sector.radius, sector.startAngle, sector.endAngle, segments, color);
}

/////////////////////////////////////////////////////////////////////////////////////////
void ReyEngine::drawLine(const Line<int>& line, float lineThick, const ReyEngine::ColorRGBA& color) {
   DrawLineEx(line.a, line.b, lineThick, color);
}

/////////////////////////////////////////////////////////////////////////////////////////
void ReyEngine::drawTexture(const ReyTexture& texture, const Rect<int> &source, const Rect<int> &dest, float rotation, const ReyEngine::ColorRGBA &tint) {
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
ReyEngine::RenderTarget::RenderTarget(){}
ReyEngine::RenderTarget::~RenderTarget() {
   if (_texLoaded) {
      UnloadRenderTexture(_tex);
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
void ReyEngine::RenderTarget::setSize(const Size<int> &newSize) {
   if (_texLoaded) {
      UnloadRenderTexture(_tex);
   }
   _tex = LoadRenderTexture(newSize.x, newSize.y);
   _texLoaded = true;
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
template<> Circle ReyEngine::Rect<double>::circumscribe() {return {{x + width / 2, y+height/2}, height/0.70710678118/2};}
template<> Circle ReyEngine::Rect<int>::circumscribe() {return {{x + width / 2, y+height/2}, height/0.70710678118/2};}
template<> Circle ReyEngine::Rect<float>::circumscribe() {return {{x + width / 2, y+height/2}, height/0.70710678118/2};}
template<> Circle ReyEngine::Rect<double>::inscribe() {return {{x + width / 2, y+height/2}, (double)height/2};}
template<> Circle ReyEngine::Rect<int>::inscribe() {return {{x + width / 2, y+height/2}, (double)height/2};}
template<> Circle ReyEngine::Rect<float>::inscribe() {return {{x + width / 2, y+height/2}, (double)height/2};}

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
ReyEngineFont ReyEngine::getDefaultFont(std::optional<float> fontSize) {
   auto retval = ReyEngineFont();
   if (fontSize){
      retval.size = fontSize.value();
   }
   return retval;
}

/////////////////////////////////////////////////////////////////////////////////////////
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

/////////////////////////////////////////////////////////////////////////////////////////
void CameraTransform::push() const {
   rlPushMatrix();
   //push camera translation matrix
   rlTranslatef(offset.x, offset.y, offset.z);
   rlScalef(zoom, zoom, 0); // do we need the Z zoom value? idk. will find out in the future when doing 3d i guess.
}

/////////////////////////////////////////////////////////////////////////////////////////
void CameraTransform::pop(){
   rlPopMatrix();
}