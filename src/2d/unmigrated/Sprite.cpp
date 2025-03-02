#include "Sprite.h"

using namespace ReyEngine;
using namespace std;

/////////////////////////////////////////////////////////////////////////////////////////
Sprite &Sprite::operator=(const Sprite &other) {
   region = other.region;
   texture = other.texture;
   texPath = other.texPath;
   _fitNextTexture = other._fitNextTexture;
   _drawDebugRect = other._drawDebugRect;
}

/////////////////////////////////////////////////////////////////////////////////////////
void Sprite::render2D() const {
   if (texture) {
      drawTexture(*texture, region, getRect().toSizeRect(), 0.0, Colors::none);
   }
   if (_drawDebugRect){
      drawRectangleLines(getRect().toSizeRect(), 1.0, Colors::blue);
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
void Sprite::registerProperties() {

}

/////////////////////////////////////////////////////////////////////////////////////////
bool Sprite::setTexture(ReyEngine::FileSystem::File& path) {
   //try load
   if (path.exists()){
      texture = make_shared<ReyTexture>(path);
      if (!region){
         region.value.setSize(texture->size);
         applyRect(region);
      } else if (_fitNextTexture){
         region = Rect<double>({0, 0}, texture->size);
         _fitNextTexture = false;
      }
   } else {
      Logger::error() << "Sprite " << getName() << " 'setTexture' failed: Path does not exist: " << path.abs() << endl;
      return false;
   }
   return true;
}

/////////////////////////////////////////////////////////////////////////////////////////
std::optional<const std::shared_ptr<ReyTexture>> Sprite::getTexture() {
   if (texture){
      return texture;
   }
   return nullopt;
}

/////////////////////////////////////////////////////////////////////////////////////////
bool Sprite::setTexture(const std::shared_ptr<ReyTexture>& other) {
   //try load
   texture = other;
   if (!region){
      region.value.setSize(texture->size);
      applyRect(region);
   } else if (_fitNextTexture){
      region = Rect<double>({0, 0}, texture->size);
      _fitNextTexture = false;
   }
   return true;
}

/////////////////////////////////////////////////////////////////////////////////////////
void Sprite::setRegion(const Rect<double>& newRegion) {
   region = newRegion;
}
/////////////////////////////////////////////////////////////////////////////////////////
void Sprite::fitTexture(){
   if (texture){
      applySize(region.value.size());
   } else {
      _fitNextTexture = true;
   }
}
/////////////////////////////////////////////////////////////////////////////////////////
void Sprite::_init() {
   if (texPath && !texture) {
      setTexture(texPath);
   }
}