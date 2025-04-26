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
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
void AnimatedSprite::render2D() const {
   if (_texture) {
      //cast to mutable reference
      auto& mutableFrameIndex = const_cast<decltype(frameIndex)&>(frameIndex);
      auto now = std::chrono::steady_clock::now();
      if (now - _ts > frameTime) {
         //set time stamp
         const_cast<std::chrono::time_point<std::chrono::steady_clock>&>(_ts) = now;
         //always verify frame index BEFORE trying to draw it - in case someone sets it to an invalid value at the wrong time.
         // for performance reasons, we are not verifying the index any other way.
         if (++mutableFrameIndex >= region.size()) {
            // if we run off the end, reset to 0;
            mutableFrameIndex = 0;
         }
      }
      drawTexture(*_texture, region[frameIndex], getSizeRect(), Colors::none);
   }
   if (_drawDebugRect){
      //draw sprite sheet visualization


      drawRectangleLines(getSizeRect(), 1.0, Colors::blue);
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
AnimatedSprite::AnimatedSprite(const FileSystem::File& texPath, const Size<R_FLOAT> &spriteSize, FrameIndex frameStart, FrameIndex frameCount)
: BaseSprite(texPath, {})
{
   //determine the region rectangles based on the given texture, sprite size, and start and end frames, assuming left->right top->bottom convention

}