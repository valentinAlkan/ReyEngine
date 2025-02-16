#include "Renderable2D.h"
#include "rlgl.h"
#include "Canvas.h"
using namespace std;
using namespace ReyEngine;
using namespace Internal;

#define SCISSOR_ERR "Cannot call scissoring functions on non-canvas objects!"
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
void Renderer2D::renderer2DChain() {
   renderer2DBegin();
   //front render - default for now
   for (const auto& child : getChildren()){
      child->renderable2DChain();
   }
   renderer2DEnd();
   renderer2DEditorFeatures();
}


/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
void Internal::Renderable2D::renderable2DChain(){
   if (!_visible) return;
   render2DBegin();

   rlPushMatrix();
   rlTranslatef(transform.position.x, transform.position.y, 0);
   rlRotatef(transform.rotation * 180/M_PI, 0,0,1);
   rlScalef(transform.scale.x, transform.scale.y, 1);

   render2D();

   //front render
   for (const auto &child: getChildren()) {
      child->renderable2DChain();
   }
   rlPopMatrix();
   render2DEnd();
   renderable2DEditorFeatures();
}

/////////////////////////////////////////////////////////////////////////////////////////
void Renderable2D::drawLine(const ReyEngine::Line<R_FLOAT>& line, float lineThick, const ReyEngine::ColorRGBA& color) const {
   ReyEngine::drawLine(line, lineThick, color);
}

/////////////////////////////////////////////////////////////////////////////////////////
void Renderable2D::drawArrow(const Line<R_FLOAT>& line, float lineThick, const ReyEngine::ColorRGBA& color, float headSize) const {
   ReyEngine::drawArrow(line, lineThick, color, headSize);
}

/////////////////////////////////////////////////////////////////////////////////////////
void Renderable2D::drawText(const std::string &text, const Pos<R_FLOAT> &pos, const ReyEngine::ReyEngineFont& font) const{
   ReyEngine::drawText(text, pos, font);
}

/////////////////////////////////////////////////////////////////////////////////////////
void Renderable2D::drawText(const std::string& text, const ReyEngine::Pos<R_FLOAT>& pos, const ReyEngine::ReyEngineFont& font, const ReyEngine::ColorRGBA& color, R_FLOAT size, R_FLOAT spacing) const {
   ReyEngine::drawText(text, pos, font, color, size, spacing);
}

/////////////////////////////////////////////////////////////////////////////////////////
void Renderable2D::drawTextCentered(const std::string &text, const Pos<R_FLOAT> &pos) const{
   ReyEngine::drawTextCentered(text, pos, theme->font);
}

/////////////////////////////////////////////////////////////////////////////////////////
void Renderable2D::drawTextCentered(const std::string &text, const Pos<R_FLOAT> &pos, const ReyEngine::ReyEngineFont& font) const{
   ReyEngine::drawTextCentered(text, pos, font);
}

/////////////////////////////////////////////////////////////////////////////////////////
void Renderable2D::drawTextCentered(const std::string &text, const ReyEngine::Pos<float> &pos, const ReyEngine::ReyEngineFont &font, const ReyEngine::ColorRGBA &color, float size, float spacing) const {
   ReyEngine::drawTextCentered(text, pos, font, color, size, spacing);
}


/////////////////////////////////////////////////////////////////////////////////////////
void Renderable2D::drawRectangle(const ReyEngine::Rect<R_FLOAT> &rect, const ReyEngine::ColorRGBA &color) const {
   ReyEngine::drawRectangle(rect, color);
}

/////////////////////////////////////////////////////////////////////////////////////////
void Renderable2D::drawRectangleLines(const ReyEngine::Rect<R_FLOAT> &rect, float lineThick, const ReyEngine::ColorRGBA &color) const {
   ReyEngine::drawRectangleLines(rect, lineThick, color);
}

/////////////////////////////////////////////////////////////////////////////////////////
void Renderable2D::drawRectangleRounded(const ReyEngine::Rect<R_FLOAT> &rect, float roundness, int segments, const ReyEngine::ColorRGBA &color) const {
   //use the size of the param rect but use the position of our rect + the param rect
//   Rect<R_FLOAT> newRect(rect + Pos<R_FLOAT>(getGlobalPos()) + Pos<R_FLOAT>(_renderOffset));
   ReyEngine::drawRectangleRounded(rect, roundness, segments, color);
}

/////////////////////////////////////////////////////////////////////////////////////////
void Renderable2D::drawRectangleRoundedLines(const ReyEngine::Rect<R_FLOAT> &rect, float roundness, int segments, float lineThick, const ReyEngine::ColorRGBA &color) const {
   //use the size of the param rect but use the position of our rect + the param rect
//   Rect<float> newRect(rect + Pos<R_FLOAT>(getGlobalPos()) + Pos<R_FLOAT>(_renderOffset));
   ReyEngine::drawRectangleRoundedLines(rect, roundness, segments, lineThick, color);
}

/////////////////////////////////////////////////////////////////////////////////////////
void Renderable2D::drawRectangleGradientV(const ReyEngine::Rect<R_FLOAT> &rect, const ReyEngine::ColorRGBA& color1, const ReyEngine::ColorRGBA &color2) const {
   //use the size of the param rect but use the position of our rect + the param rect
//   Rect<float> newRect(rect + getGlobalPos() + _renderOffset);
   ReyEngine::drawRectangleGradientV(rect, color1, color2);
}

///////////////////////////////////////////////////////////////////////////////////////
void Renderable2D::drawCircle(const ReyEngine::Circle& circle, const ReyEngine::ColorRGBA& color) const {
   ReyEngine::drawCircle(circle, color);
}

///////////////////////////////////////////////////////////////////////////////////////
void Renderable2D::drawCircleLines(const ReyEngine::Circle& circle, const ReyEngine::ColorRGBA& color) const {
   ReyEngine::drawCircleLines(circle, color);
}

///////////////////////////////////////////////////////////////////////////////////////
void Renderable2D::drawCircleSectorLines(const ReyEngine::CircleSector& sector, const ReyEngine::ColorRGBA& color, int segments) const {
   ReyEngine::drawCircleSectorLines(sector, color, segments);
}

///////////////////////////////////////////////////////////////////////////////////////
void Renderable2D::drawRenderTarget(const ReyEngine::RenderTarget& target, const Pos<R_FLOAT>& dst, const ColorRGBA& tint) const {
   DrawTextureRec(target.getRenderTexture(), {0, 0, (float)target.getSize().x, -(float)target.getSize().y}, {dst.x, dst.y}, tint);
}

///////////////////////////////////////////////////////////////////////////////////////
void Renderable2D::drawRenderTargetRect(const ReyEngine::RenderTarget& target, const Rect<R_FLOAT>& src, const Rect<R_FLOAT>& dst, const ColorRGBA& tint) const {
   Rectangle _src = {src.x, -src.y-src.height, (float)src.width, -(float)src.height};
   Rectangle _dst = {dst.x, dst.y, dst.width, dst.height};
   DrawTexturePro(target.getRenderTexture(), _src, _dst, {}, 0, tint);
}

///////////////////////////////////////////////////////////////////////////////////////
void Renderable2D::drawTextureRect(const ReyEngine::ReyTexture& rtex, const ReyEngine::Rect<R_FLOAT> &src, const ReyEngine::Rect<R_FLOAT> &dst, float rotation, const ReyEngine::ColorRGBA &tint) const {
   Rectangle _src  = {src.x, src.y, src.width, src.height};
   Rectangle _dst = {dst.x, dst.y, dst.width, dst.height};
   DrawTexturePro(rtex.getTexture(), _src, _dst, {0,0}, rotation, Colors::none);
}