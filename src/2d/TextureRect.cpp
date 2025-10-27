#include "TextureRect.h"
#include "Application.h"

using namespace std;
using namespace ReyEngine;

///////////////////////////////////////////////////////////////////////////////////////
void TextureRect::_init() {
   if (_fitScheduled && _texture){
      setRect({getPos(), _texture->size});
      _fitScheduled = false;
   }
}

///////////////////////////////////////////////////////////////////////////////////////
void TextureRect::setTexture(const FileSystem::File& _newPath) {
   if (Application::isReady()) {
      _texture = make_shared<ReyTexture>(_newPath);
   }
}

///////////////////////////////////////////////////////////////////////////////////////
void TextureRect::setTexture(const shared_ptr<ReyTexture>& newTexture){
    if (Application::isReady()) {
        _texture = newTexture;
    }
}

///////////////////////////////////////////////////////////////////////////////////////
void TextureRect::setTexture(ReyTexture&& newTexture){
   if (Application::isReady() && newTexture) {
      *_texture = std::move(newTexture);
   }
}

///////////////////////////////////////////////////////////////////////////////////////
template <>
void Internal::DrawArea<TextureRect>::fitTexture() {
   if (_has_inited) {
      if (_texture) {
         setRect({getPos(), _texture->size});
      }
   } else{
      _fitScheduled = true;
   }
}