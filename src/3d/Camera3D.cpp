#include "Camera3D.h"

/////////////////////////////////////////////////////////////////////////////////////////
void ReyEngine::Camera3D::renderer3DBegin() {
   //zero out our position on the texture offset
   //pop the active camera - that is to say, return to a 'no camera' scenario
   _camera.pop();
}

/////////////////////////////////////////////////////////////////////////////////////////
void ReyEngine::Camera3D::renderer3DEnd() {
   //return to trasnformation mode
   _camera.push();
}