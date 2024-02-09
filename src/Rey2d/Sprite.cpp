#include "Sprite.h"

using namespace ReyEngine;
using namespace std;

/////////////////////////////////////////////////////////////////////////////////////////
void Sprite::render() const {
   if (texture) {
      auto rect = getGlobalRect();
      drawTexture(*texture, region, rect, 0.0, Colors::none);
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
void Sprite::registerProperties() {

}

/////////////////////////////////////////////////////////////////////////////////////////
bool Sprite::setTexture(const ReyEngine::FileSystem::File& path, const ReyEngine::Rect<int>& drawRegion) {
   //try load
   if (path.exists()){
      texture = make_unique<ReyTexture>(path);
      region = drawRegion;
      //if region is undefined, set the region to the max size of the texture
      if (!region){
         region.value.setSize(texture->size);
      }
   } else {
      cerr << "setTexture failed: Path does not exist: " << path << endl;
      return false;
   }
   return true;
}
/////////////////////////////////////////////////////////////////////////////////////////
void Sprite::fitTexture(){
   if (texture){
      _rect.value.setSize(region.value.size());
   }
}
/////////////////////////////////////////////////////////////////////////////////////////
void Sprite::_init() {
   if (texPath && !texture) {
      texture = make_unique<ReyTexture>(texPath);
   }
}