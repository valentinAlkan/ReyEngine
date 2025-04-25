#include "Sprite.h"

using namespace ReyEngine;
using namespace std;

/////////////////////////////////////////////////////////////////////////////////////////
void Sprite::render2D() const {
   if (_texture) {
      drawTexture(*_texture, region, getSizeRect(), Colors::none);
   }
   if (_drawDebugRect){
      drawRectangleLines(getSizeRect(), 1.0, Colors::blue);
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
void Sprite::_init(){
   if (texPath && !_texture) {
      setTexture(texPath);
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
bool Sprite::setTexture(FileSystem::File& path){ //no temporaries!
   //try load
   if (path.exists()){
      setTexture(std::make_shared<ReyTexture>(path));
   } else {
      Logger::error() << "Sprite " << getName() << " 'setTexture' failed: Path does not exist: " << path.abs() << std::endl;
      return false;
   }
   return true;
};

/////////////////////////////////////////////////////////////////////////////////////////
bool Sprite::setTexture(const shared_ptr<ReyTexture>& other) {
   _texture = other;
   if (!isValidRegion()){
      region.setSize(_texture->size);
      applyRect(region);
   } else if (_fitNextTexture){
      region = Rect<double>({0, 0}, _texture->size);
      _fitNextTexture = false;
   }
   return true;
}

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
void AnimatedSprite::render2D() const {
   if (_texture) {
      //cast to mutable reference
      auto& mutableFrameIndex = const_cast<decltype(frameIndex)&>(frameIndex);
      //always verify frame index BEFORE trying to draw it - in case someone sets it at the wrong time
      if (frameIndex>=region.size()){
         mutableFrameIndex=0;
      }
      drawTexture(*_texture, region[frameIndex], getSizeRect(), Colors::none);
      //increment frameCounter and then verify that it is not off the end

   }
   if (_drawDebugRect){
      drawRectangleLines(getSizeRect(), 1.0, Colors::blue);
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
AnimatedSprite::AnimatedSprite(const FileSystem::File& texPath, Size<R_FLOAT> spriteSize, FrameIndex frameStart, FrameIndex frameCount)
: BaseSprite(texPath, {})
{
   //determine the region rectangles based on the given texture, sprite size, and start and end frames, assuming left->right top->bottom convention

}