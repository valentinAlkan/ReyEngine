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
bool Sprite::setTexture(const ReyEngine::FileSystem::File& path) {
   //try load
   if (path.exists()){
      texture = make_unique<ReyTexture>(path);
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