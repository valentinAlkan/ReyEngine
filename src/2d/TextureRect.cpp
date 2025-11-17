#include "TextureRect.h"
#include "Application.h"

using namespace std;
using namespace ReyEngine;

///////////////////////////////////////////////////////////////////////////////////////
void TextureRect::_init() {
   if (_fitScheduled && _texture){
      setRect({getPos(), _texture->size()});
      _fitScheduled = false;
   }
}

///////////////////////////////////////////////////////////////////////////////////////
void TextureRect::setTexture(const FileSystem::File& _newPath) {
   if (Application::isReady()) {
      _texture = make_shared<ReyTexture>(_newPath);
      _region = _texture->size().toRect();
      _calculateFit();
   }
}

///////////////////////////////////////////////////////////////////////////////////////
void TextureRect::setTexture(const shared_ptr<ReyTexture>& newTexture){
    if (Application::isReady()) {
        _texture = newTexture;
       _region = _texture->size().toRect();
       _calculateFit();
    }
}