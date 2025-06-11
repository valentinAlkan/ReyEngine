#include "TextureRect.h"
#include "Application.h"

using namespace std;
using namespace ReyEngine;

///////////////////////////////////////////////////////////////////////////////////////
void TextureRect::render2D() const {
   if (_texture) {
      Rect<float> srcRect = {{0, 0}, _texture->size};
      Rect<float> dstRect = getSizeRect();
      switch(_fitType) {
         case FitType::FIT_RECT: break;
         case FitType::FIT_WIDTH:
            dstRect.height = srcRect.height;
            break;
         case FitType::FIT_HEIGHT:
            dstRect.width = srcRect.width;
            break;
         case FitType::NONE:
            srcRect = dstRect;
            break;
      }
      drawTexture(*_texture, srcRect, dstRect, Colors::none);
   } else {
      auto sizeRect = getSizeRect();
      drawRectangleLines(sizeRect, 2.0, Colors::red);
      drawLine({sizeRect.topLeft(), sizeRect.bottomRight()}, 2.0, Colors::red);
      drawLine({sizeRect.bottomLeft(), sizeRect.topRight()}, 2.0, Colors::red);
   }
}

///////////////////////////////////////////////////////////////////////////////////////
void TextureRect::_init() {
   if (!_texturePath.empty() && !_texture){
      setTexture(_texturePath);
   }
   if (_fitScheduled && _texture){
      setRect({getPos(), _texture->size});
      _fitScheduled = false;
   }
}

///////////////////////////////////////////////////////////////////////////////////////
void TextureRect::setTexture(const FileSystem::File& _newPath) {
   _texturePath = _newPath.str();
   if (Application::isReady()) {
      _texture = make_shared<ReyTexture>(_newPath);
   }
}

///////////////////////////////////////////////////////////////////////////////////////
void TextureRect::setTexture(const shared_ptr<ReyTexture>& newTexture){
    _texturePath = newTexture->getPath();
    if (Application::isReady()) {
        _texture = newTexture;
    } 
}

///////////////////////////////////////////////////////////////////////////////////////
void TextureRect::fitTexture() {
   if (_has_inited) {
      if (_texture) {
         setRect({getPos(), _texture->size});
      }
   } else{
      _fitScheduled = true;
   }
}