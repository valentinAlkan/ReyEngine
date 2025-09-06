#include "ReyEngine.h"
#include "rlgl.h"
#include <cstring>

using namespace ReyEngine;
using namespace std;

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
ReyTexture::ReyTexture(const FileSystem::File& file)
{
   loadTexture(file);
}

/////////////////////////////////////////////////////////////////////////////////////////
ReyTexture::ReyTexture(const ReyImage& image)
{
   _tex = LoadTextureFromImage(image._image);
   size = {image._image.width, image._image.height};
   _texLoaded = true;
}

/////////////////////////////////////////////////////////////////////////////////////////
ReyTexture::ReyTexture(ReyImage&& image)
{
   _tex = LoadTextureFromImage(image._image);
   size = {image._image.width, image._image.height};
   _texLoaded = true;
}

/////////////////////////////////////////////////////////////////////////////////////////
ReyTexture& ReyTexture::operator=(ReyEngine::ReyImage&& image){
   _tex = LoadTextureFromImage(image._image);
   size = {image._image.width, image._image.height};
   _texLoaded = true;
   return *this;
}

/////////////////////////////////////////////////////////////////////////////////////////
void ReyTexture::loadTexture(const FileSystem::File &file) {
   auto path = file.abs();
   if (!file.exists()){
      Logger::error() << "LoadTexture failure: Texture file " << file.abs() << " does not exist!" << endl;
   }
   _tex = LoadTexture(path.c_str());
   _texLoaded = true;
   size = {_tex.width, _tex.height};
}
/////////////////////////////////////////////////////////////////////////////////////////
template<> Rect<float> Pos<float>::toRect() const {return {x,y,0,0};}
template<> Rect<double> Pos<double>::toRect() const {return {x,y,0,0};}
template<> Rect<int> Pos<int>::toRect() const {return {x,y,0,0};}
/////////////////////////////////////////////////////////////////////////////////////////
template<> Rect<float> Pos<float>::toRect(const Size<float>& s) const {return {{x, y}, {s}};}
template<> Rect<double> Pos<double>::toRect(const Size<double>& s) const {return {{x, y}, {s}};}
template<> Rect<int> Pos<int>::toRect(const Size<int>& s) const {return {{x, y}, {s}};}
/////////////////////////////////////////////////////////////////////////////////////////
template<> Rect<float> Pos<float>::toCenterRect(const Size<float>& s) const {return {*this-Pos<float>(s/2),{s}};}
template<> Rect<double> Pos<double>::toCenterRect(const Size<double>& s) const {return {*this-Pos<double>(s/2),{s}};}
template<> Rect<int> Pos<int>::toCenterRect(const Size<int>& s) const {return {*this-Pos<int>(s/2),{s}};}
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
CanvasSpace<Pos<R_FLOAT>> ReyEngine::getScreenCenter() {
   return {{(float)(GetScreenWidth())/2, (float)(GetScreenHeight())/2}};
}

/////////////////////////////////////////////////////////////////////////////////////////
Size<int> ReyEngine::getScreenSize() {
   return {GetScreenWidth(),GetScreenHeight()};
}

/////////////////////////////////////////////////////////////////////////////////////////
void ReyEngine::drawText(const std::string &text, const Pos<R_FLOAT>& pos, const ReyEngineFont& font) {
   drawText(text, pos, font, font.color, font.size, font.spacing);
}

/////////////////////////////////////////////////////////////////////////////////////////
void ReyEngine::drawText(const std::string& text, const Pos<R_FLOAT>& pos, const ReyEngine::ReyEngineFont& font, const ReyEngine::ColorRGBA& color, R_FLOAT size, R_FLOAT spacing) {
   DrawTextPro(font.font, text.c_str(), {(float)pos.x, (float)pos.y}, {0, 0}, 0, size, spacing, color);
}

/////////////////////////////////////////////////////////////////////////////////////////
void ReyEngine::drawRectangle(const Rect<R_FLOAT>& r, const ReyEngine::ColorRGBA& color) {
   DrawRectangle((int)r.x, (int)r.y, (int)r.width, (int)r.height, color);
}

/////////////////////////////////////////////////////////////////////////////////////////
void ReyEngine::drawRectangleRounded(const Rect<R_FLOAT>& r, float roundness, int segments, const ReyEngine::ColorRGBA& color) {
   DrawRectangleRounded({r.x, r.y, r.width, r.height}, roundness, segments, color);
}


/////////////////////////////////////////////////////////////////////////////////////////
void ReyEngine::drawRectangleLines(const Rect<R_FLOAT>& r, float lineThick, const ReyEngine::ColorRGBA& color) {
   DrawRectangleLinesEx({r.x, r.y, r.width, r.height}, lineThick, color);
}

/////////////////////////////////////////////////////////////////////////////////////////
void ReyEngine::drawRectangleRoundedLines(const Rect<R_FLOAT>& r, float roundness, int segments, float lineThick, const ReyEngine::ColorRGBA& color) {
   DrawRectangleRoundedLines({r.x, r.y, r.width, r.height}, roundness, segments, lineThick, color);
}

/////////////////////////////////////////////////////////////////////////////////////////
void ReyEngine::drawRectangleGradientV(const Rect<R_FLOAT>& rect, const ReyEngine::ColorRGBA& color1, const ReyEngine::ColorRGBA& color2) {
   DrawRectangleGradientV((int)rect.x, (int)rect.y, (int)rect.width, (int)rect.height, color1, color2);
}

void ReyEngine::drawRectangleGradientH(const Rect<R_FLOAT>& rect, const ReyEngine::ColorRGBA& color1, const ReyEngine::ColorRGBA& color2) {
   DrawRectangleGradientH((int)rect.x, (int)rect.y, (int)rect.width, (int)rect.height, color1, color2);
}

/////////////////////////////////////////////////////////////////////////////////////////
void ReyEngine::drawCircle(const Circle& circle, const ReyEngine::ColorRGBA &color) {
   DrawCircle((int)circle.center.x, (int)circle.center.y, circle.radius, color);
}

/////////////////////////////////////////////////////////////////////////////////////////
void ReyEngine::drawCircleLines(const Circle& circle, const ReyEngine::ColorRGBA &color) {
   DrawCircleLines((int)circle.center.x, (int)circle.center.y, circle.radius, color);
}

/////////////////////////////////////////////////////////////////////////////////////////
void ReyEngine::drawCircleSector(const CircleSector& sector, const ReyEngine::ColorRGBA &color, int segments) {
   DrawCircleSector((Vector2)sector.center, sector.radius, (float)sector.startAngle, (float)sector.endAngle, segments, color);
}

/////////////////////////////////////////////////////////////////////////////////////////
void ReyEngine::drawCircleSectorLines(const CircleSector& sector, const ReyEngine::ColorRGBA &color, int segments) {
   DrawCircleSectorLines((Vector2)sector.center, sector.radius, sector.startAngle, sector.endAngle, segments, color);
}

/////////////////////////////////////////////////////////////////////////////////////////
void ReyEngine::drawLine(const Line<R_FLOAT>& line, float lineThick, const ReyEngine::ColorRGBA& color) {
   DrawLineEx((Vector2)line.a, (Vector2)line.b, lineThick, color);
}

/////////////////////////////////////////////////////////////////////////////////////////
void ReyEngine::drawArrow(const Line<R_FLOAT>& line, float lineThick, const ReyEngine::ColorRGBA &color, float headSize) {
    DrawLineEx((Vector2)line.a, (Vector2)line.b, lineThick, color);
    static constexpr auto rotation = 25_deg;
    auto _line = line;
    auto pctLine = _line.project(headSize);
    auto headline = pctLine.rotate(pctLine.a, rotation);
    DrawLineEx((Vector2)headline.a, (Vector2)headline.b, lineThick, color);
    headline = pctLine.rotate(pctLine.a, -rotation);
    DrawLineEx((Vector2)headline.a, (Vector2)headline.b, lineThick, color);
}

/////////////////////////////////////////////////////////////////////////////////////////
void ReyEngine::drawArrowHead(const Line<R_FLOAT>& line, float lineThick, const ReyEngine::ColorRGBA &color, float headSize) {
//   DrawLineEx((Vector2)line.a, (Vector2)line.b, lineThick, color);
//   static constexpr auto rotation = 25_deg;
//   auto pctLine = line.project(headSize);
//   auto headline = pctLine.rotate(pctLine.a, rotation);
//   DrawLineEx((Vector2)headline.a, (Vector2)headline.b, lineThick, color);
//   headline = pctLine.rotate(pctLine.a, -rotation);
//   DrawLineEx((Vector2)headline.a, (Vector2)headline.b, lineThick, color);
}

/////////////////////////////////////////////////////////////////////////////////////////
void ReyEngine::drawTextCentered(const std::string& text, const Pos<R_FLOAT>& pos, const ReyEngineFont& font){
   drawTextCentered(text, pos, font, font.color, font.size, font.spacing);
}

/////////////////////////////////////////////////////////////////////////////////////////
void ReyEngine::drawTextCentered(const std::string &text, const Pos<float> &pos, const ReyEngineFont &font, const ReyEngine::ColorRGBA &color, float size, float spacing) {
   auto textWidth = MeasureText(text.c_str(), size);
   float newX = (float)pos.x - (float)textWidth / 2;
   float newY = (float)pos.y - (float)size / 2;
   drawText(text, {newX, newY}, font, color, size, spacing);
}


/////////////////////////////////////////////////////////////////////////////////////////
void ReyEngine::drawTextRelative(const std::string& text, const Pos<R_FLOAT>& relPos, const ReyEngineFont& font){
   //draw text relative as a percentage of the screen
   Vec2<R_FLOAT> screenSize = {(float)GetScreenWidth(), (float)GetScreenHeight()};
   auto newX = (float)(screenSize.x * relPos.x / 100.0);
   auto newY = (float)(screenSize.y * relPos.y / 100.0);
   drawText(text, {newX, newY}, font);
}

/////////////////////////////////////////////////////////////////////////////////////////
void ReyEngine::drawTexture(const ReyTexture& texture, const Rect<R_FLOAT> &source, const Rect<R_FLOAT> &dest, const ReyEngine::ColorRGBA &tint) {
   //Note: this is not y-flipped. That's a renderTexture thing.
   DrawTexturePro(texture.getTexture(), source, dest, {0,0},  0, tint);
}

/////////////////////////////////////////////////////////////////////////////////////////
void ReyEngine::drawRenderTargetRect(const RenderTarget& target, const Rect<R_FLOAT>& src, const Rect<R_FLOAT>& dst, const ColorRGBA& tint){
   // NOTE: Render texture must be y-flipped due to default OpenGL coordinates (left-bottom)
   // NOTE: Using DrawTexturePro() we can easily rotate and scale the part of the texture we draw
   // sourceRec defines the part of the texture we use for drawing
   // destRec defines the rectangle where our texture part will fit (scaling it to fit)
   // origin defines the point of the texture used as reference for rotation and scaling
   // rotation defines the texture rotation (using origin as rotation point)
   // Flip the Y coordinates of the source rectangle
   // NOTE: don't understand the math but it seems to work.  ¯\_(ツ)_/¯
   auto newSrc = src;
   newSrc.y = (float)target.getTexture().height - newSrc.y - newSrc.height;
   newSrc.height = -newSrc.height;
   DrawTexturePro(target.getTexture(), newSrc, dst, {0, 0}, 0, tint);
}

/////////////////////////////////////////////////////////////////////////////////////////
void ReyEngine::drawRenderTarget(const RenderTarget& target, const Pos<R_FLOAT>& pos, const ColorRGBA& tint){
   auto rect = pos.toRect(target.getSize());
   drawRenderTargetRect(target, rect, rect, tint);
}

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
ReyEngine::RenderTarget::RenderTarget(){
   // make sure you initialize at some point
}

ReyEngine::RenderTarget::RenderTarget(const Size<int>& size) {
   setSize(size);
}
/////////////////////////////////////////////////////////////////////////////////////////
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
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
void ReyEngine::setWindowSize(ReyEngine::Size<int> size) {
   SetWindowSize(size.x, size.y);
}

/////////////////////////////////////////////////////////////////////////////////////////
Size<int> ReyEngine::getWindowSize() {
   return {GetRenderWidth(), GetRenderHeight()};
}

/////////////////////////////////////////////////////////////////////////////////////////
Pos<int> ReyEngine::getWindowPosition() {
   return GetWindowPosition();
}

/////////////////////////////////////////////////////////////////////////////////////////
void ReyEngine::setWindowPosition(Pos<int> pos) {
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
template<> Circle Rect<double>::circumscribe() const {return {{(R_FLOAT)(x + width / 2), (R_FLOAT)(y+height/2)}, static_cast<float>((R_FLOAT)height/0.70710678118/2.0)};}
template<> Circle Rect<int>::circumscribe() const {return {{(R_FLOAT)(x + width / 2), (R_FLOAT)(y+height/2)}, static_cast<float>(((R_FLOAT)height)/0.70710678118/2.0)};}
template<> Circle Rect<float>::circumscribe() const {return {{(R_FLOAT)(x + width / 2), (R_FLOAT)(y+height/2)}, static_cast<float>(((R_FLOAT)height)/0.70710678118/2.0)};}
template<> Circle Rect<double>::inscribe() const {return {{(R_FLOAT)(x + width / 2), (R_FLOAT)(y+height/2)}, (R_FLOAT)height/2};}
template<> Circle Rect<int>::inscribe() const {return {{(R_FLOAT)(x + width / 2), (R_FLOAT)(y+height/2)}, (R_FLOAT)height/2};}
template<> Circle Rect<float>::inscribe() const {return {{(R_FLOAT)(x + width / 2), (R_FLOAT)(y+height/2)}, (R_FLOAT)height/2};}
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
ReyEngineFont ReyEngine::getDefaultFont(std::optional<R_FLOAT> fontSize) {
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
Size<R_FLOAT> ReyEngineFont::measure(const std::string &text) const {
   return ReyEngine::measureText(text, *this);
}

/////////////////////////////////////////////////////////////////////////////////////////
template <>
UnitVector2 Vec2<R_FLOAT>::direction(const Vec2<float> &dest) const {
   return {dest - *this};
}

/////////////////////////////////////////////////////////////////////////////////////////
CircleSector UnitVector2::toCircleSector(Degrees totalAngle, double radius, const Pos<R_FLOAT> &pos) const {
   const double startAngle = std::fmod(((std::atan2(_x, _y) * 180.0 / M_PI) - (totalAngle.get() / 2) + 360.0),360.0);
   const double tempEndAngle = std::fmod(((std::atan2(_x, _y) * 180.0 / M_PI) + (totalAngle.get() / 2) + 360.0),360.0);
   //wrap around 360
   const double endAngle = tempEndAngle + (tempEndAngle < startAngle ? 360.0 : 0.0);
   return {pos, radius, startAngle, endAngle};
}
