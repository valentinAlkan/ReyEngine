#include "Sprite.h"

using namespace ReyEngine;
using namespace std;

/////////////////////////////////////////////////////////////////////////////////////////
void Sprite::render() const {
   if (texture) {
      auto rect = getGlobalRect();
      drawTexture(*texture, region, rect, 0.0, Colors::none);
   }
   if (_drawDebugRect){
      drawRectangleLines(getRect().toSizeRect(), 1.0, Colors::blue);
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
void Sprite::registerProperties() {

}

/////////////////////////////////////////////////////////////////////////////////////////
bool Sprite::setTexture(const ReyEngine::FileSystem::File& path) {
   //try load
   if (path.exists()){
      texture = make_unique<ReyTexture>(path);
      if (!region){
         region.value.setSize(texture->size);
         _rect = region;
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
std::optional<const std::reference_wrapper<ReyTexture>> Sprite::getTexture() {
   if (texture){
      return *texture;
   }
   return nullopt;
}

/////////////////////////////////////////////////////////////////////////////////////////
void Sprite::setRegion(const Rect<double>& newRegion) {
   region = newRegion;
}
/////////////////////////////////////////////////////////////////////////////////////////
void Sprite::fitTexture(){
   if (texture){
      _rect.value.setSize(region.value.size());
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