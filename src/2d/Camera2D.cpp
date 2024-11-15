#include "Camera2D.h"
#include "rlgl.h"

using namespace std;
using namespace ReyEngine;

/////////////////////////////////////////////////////////////////////////////////////////
void ReyEngine::Camera2D::renderBegin(ReyEngine::Pos<R_FLOAT> &textureOffset) {
   //zero out our position on the texture offset
   textureOffset -= getPos();
   //pop the active camera - that is to say, return to a 'no camera' scenario
   _camera.pop();
}

/////////////////////////////////////////////////////////////////////////////////////////
void ReyEngine::Camera2D::renderEnd() {
   //return to trasnformation mode
   _camera.push();
}