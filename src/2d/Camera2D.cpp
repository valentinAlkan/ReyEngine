#include "Camera2D.h"
#include "rlgl.h"

using namespace std;
using namespace ReyEngine;

/////////////////////////////////////////////////////////////////////////////////////////
void ReyEngine::Camera2D::render2DBegin() {
   //pop the active camera - that is to say, return to a 'no camera' scenario
   rlDrawRenderBatchActive();
   rlPushMatrix();
}

/////////////////////////////////////////////////////////////////////////////////////////
void ReyEngine::Camera2D::render2DEnd() {
   //return to transformation mode;
   rlDrawRenderBatchActive();
   rlPopMatrix();
}

/////////////////////////////////////////////////////////////////////////////////////////
std::optional<std::shared_ptr<BaseWidget>> ReyEngine::Camera2D::askHover(const ReyEngine::Pos<float> &globalPos) {
//   return BaseWidget::askHover(InputManager::getMousePos() + getGlobalPos().get());
}

/////////////////////////////////////////////////////////////////////////////////////////
Handled ReyEngine::Camera2D::__process_unhandled_input(const InputEvent& event, const std::optional<UnhandledMouseInput>& mouse){
   NOT_IMPLEMENTED;
//   //for mouse events, convert global coordinates to world space, then pass along the normal chain
//   std::optional<UnhandledMouseInput> screenSpaceMouse = mouse;
//   if (mouse){
//      screenSpaceMouse.value().localPos = InputManager::getMousePos();
//      screenSpaceMouse->isInside = isInside(screenSpaceMouse.value().localPos);
//   }
//
//   switch (event.eventId){
//      case InputEventMouseMotion::getUniqueEventId():
//      case InputEventMouseButton::getUniqueEventId():
//      case InputEventMouseWheel::getUniqueEventId():{
//         //have to make sure we store enough memory to copy correctly - we won't know the size in advance
//         union InputEventUnion {
//            InputEventMouseMotion motion;
//            InputEventMouseButton button;
//            InputEventMouseWheel wheel;
//         };
//         char raw[sizeof(InputEventUnion)];
//         //just go ahead and copy off the end, we don't really care what's there
//         memcpy(raw, &event, sizeof(InputEventUnion));
//         auto& _screenSpaceEvent = reinterpret_cast<InputEventMouse&>(raw);
//         _screenSpaceEvent.canvasPos = InputManager::getMousePos() + getGlobalPos().get();
//         return _process_unhandled_input(reinterpret_cast<InputEvent&>(raw), screenSpaceMouse);
//      }
//   }
//   return _process_unhandled_input(event, mouse);
}


/////////////////////////////////////////////////////////////////////////////////////////
void ReyEngine::Camera2D ::render2DChain(Pos<R_FLOAT>& parentOffset) {
   static std::vector<Matrix> frameStack;
   auto getPointInFrame = [](Vector3 point, const Matrix& frameMatrix) {
      return Vector3Transform(point, MatrixInvert(frameMatrix));
   };

   if (!_visible) return;
   Pos<R_FLOAT> localOffset;
   render2DBegin();
   //backrender

   auto rotation = Degrees(getRotation()).get();

   rlLoadIdentity();
   if (transform.position.x || transform.position.y || rotation) {
      frameStack.push_back(MatrixRotate({0,0,1}, rotation));
   }
   for (const auto &child: _backRenderList) {
      child->render2DChain();
   }

   if (!frameStack.empty()) {
      drawLine({{-transform.position.x, -transform.position.y}, {0, 0}}, 2.0, Colors::red);
   }
   render2D();

   //front render
   for (const auto &child: _frontRenderList) {
      child->render2DChain();
   }
   if (!frameStack.empty()) {
      frameStack.pop_back();
   }

   render2DEnd();
   render2DEditorFeatures();
}