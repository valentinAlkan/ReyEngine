#include "Camera2D.h"
#include "rlgl.h"

using namespace std;
using namespace ReyEngine;

/////////////////////////////////////////////////////////////////////////////////////////
void ReyEngine::Camera2D::renderBegin(ReyEngine::Pos<R_FLOAT> &textureOffset) {
   //zero out our position on the texture offset
   textureOffset -= getPos();
   //pop the active camera - that is to say, return to a 'no camera' scenario
   rlDrawRenderBatchActive();
   rlPushMatrix();
}

/////////////////////////////////////////////////////////////////////////////////////////
void ReyEngine::Camera2D::renderEnd() {
   //return to transformation mode;
   rlDrawRenderBatchActive();
   rlPopMatrix();
}

/////////////////////////////////////////////////////////////////////////////////////////
std::optional<std::shared_ptr<BaseWidget>> ReyEngine::Camera2D::askHover(const ReyEngine::Pos<float> &globalPos) {
   return BaseWidget::askHover(InputManager::getMousePos() + getGlobalPos());
}

/////////////////////////////////////////////////////////////////////////////////////////
Handled ReyEngine::Camera2D::__process_unhandled_input(const InputEvent& event, const std::optional<UnhandledMouseInput>& mouse){
   //for mouse events, convert global coordinates to world space, then pass along the normal chain
   std::optional<UnhandledMouseInput> screenSpaceMouse = mouse;
   if (mouse){
      screenSpaceMouse.value().localPos = InputManager::getMousePos();
      screenSpaceMouse->isInside = isInside(screenSpaceMouse.value().localPos);
   }

   switch (event.eventId){
      case InputEventMouseMotion::getUniqueEventId():
      case InputEventMouseButton::getUniqueEventId():
      case InputEventMouseWheel::getUniqueEventId():{
         //have to make sure we store enough memory to copy correctly - we won't know the size in advance
         union InputEventUnion {
            InputEventMouseMotion motion;
            InputEventMouseButton button;
            InputEventMouseWheel wheel;
         };
         char raw[sizeof(InputEventUnion)];
         //just go ahead and copy off the end, we don't really care what's there
         memcpy(raw, &event, sizeof(InputEventUnion));
         auto& _screenSpaceEvent = reinterpret_cast<InputEventMouse&>(raw);
         _screenSpaceEvent.globalPos = InputManager::getMousePos() + getGlobalPos();
         return _process_unhandled_input(reinterpret_cast<InputEvent&>(raw), screenSpaceMouse);
      }
   }
   return _process_unhandled_input(event, mouse);
}


/////////////////////////////////////////////////////////////////////////////////////////
void ReyEngine::Camera2D ::renderChain(Pos<R_FLOAT>& parentOffset) {
   static std::vector<Matrix> frameStack;
   auto getPointInFrame = [](Vector3 point, const Matrix& frameMatrix) {
      return Vector3Transform(point, MatrixInvert(frameMatrix));
   };

   if (!_visible) return;
   Pos<R_FLOAT> localOffset;
   renderBegin(localOffset);
   auto prevOffset = _renderOffset;
   _renderOffset += (localOffset + parentOffset);
   //backrender

   auto rotation = Degrees(getRotation()).get();

   rlLoadIdentity();
   if (transform.position.x || transform.position.y || rotation) {
      frameStack.push_back(MatrixRotate({0,0,1}, rotation));
   }
   for (const auto &child: _backRenderList) {
      child->renderChain(_renderOffset);
   }

   if (!frameStack.empty()) {
      drawLine({{-transform.position.x, -transform.position.y}, {0, 0}}, 2.0, Colors::red);
   }
   render();

   //front render
   for (const auto &child: _frontRenderList) {
      child->renderChain(_renderOffset);
   }
   if (!frameStack.empty()) {
      frameStack.pop_back();
   }

   _renderOffset = prevOffset; //reset to local offset when we are done
   renderEnd();
   renderEditorFeatures();
}