#include "TextureRect.h"
#include "Application.h"

using namespace std;
using namespace ReyEngine;

///////////////////////////////////////////////////////////////////////////////////////
void TextureRect::render() const {
   if (_texture) {
      _drawTextureRect(_texture, _rect.value.toSizeRect(), _rect.value.toSizeRect(), 0.0, Colors::none);
   }
}

///////////////////////////////////////////////////////////////////////////////////////
void TextureRect::_init() {
   if (!_texturePath.value.empty() && !_texture){
      setTexture(_texturePath.value);
   }
   if (_fitScheduled && _texture){
      setRect({getPos(), _texture.size});
      _fitScheduled = false;
   }
}

///////////////////////////////////////////////////////////////////////////////////////
void TextureRect::setTexture(const FileSystem::File& _newPath) {
   _texturePath = _newPath;
   if (Application::isReady()) {
      _texture.loadTexture(_newPath);
   }
}

///////////////////////////////////////////////////////////////////////////////////////
void TextureRect::fitTexture() {
   if (_has_inited) {
      if (_texture) {
         setRect({getPos(), _texture.size});
      }
   } else{
      _fitScheduled = true;
   }
}