#pragma once
#include "BaseWidget.h"
#include "Application.h"
#include "Canvas.h"
#include "Logger.h"

namespace ReyEngine {

   //Each canvas can have excatly 1 active camera at a time!!!!
   class Camera2D : public BaseWidget {
      REYENGINE_OBJECT_CUSTOM_BUILD(Camera2D, BaseWidget, std::tuple<const std::string&, const Size<int>&>)
   public:
      void setZoom(double newZoom){transform.scale = {newZoom, newZoom};}
      double getZoom() const {return transform.scale.x;}
      void setTarget(const Pos<double>& target){ setPos(target - getSize()/2);}
      Pos<int> getTarget() const{return pos + getSize()/2;}
      /// Translates the coordinates on the camera's lens to a point on the world it is viewing
      Pos<R_FLOAT> screenToWorld(const Pos<R_FLOAT>& pos) const {
         Matrix invMatCamera = MatrixInvert(getCameraMatrix2D());
         Vector3 transform = Vector3Transform((Vector3){ pos.x, pos.y, 0 }, invMatCamera);
         return {transform.x, transform.y};
      }
      Pos<R_FLOAT> worldToscreen(const Pos<R_FLOAT>& pos) const {
         Matrix matCamera = getCameraMatrix2D();
         Vector3 transform = Vector3Transform((Vector3){ pos.x, pos.y, 0 }, matCamera);
         return (Vector2){ transform.x, transform.y };
      }
      /// We don't set the offset directly, however sometimes it's handy to know what it is. This would be mostly used for debugging purposes.
      Pos<double> getOffset() const {return transform.translation;}
      Pos<double> nearToFar(const Pos<float>& pos) const {return screenToWorld(pos);}
      Pos<double> farToNear(const Pos<float>& pos) const {return worldToscreen(pos);}
      static std::shared_ptr<Camera2D> build(const std::string& name, const Size<int>& screenSize) {
         return Camera2D::_reyengine_make_shared(name, screenSize);
      }

      Matrix getCameraMatrix2D() const
      {
         Matrix matTransform = { 0 };
         // The camera in world-space is set by
         //   1. Move it to target
         //   2. Rotate by -rotation and scale by (1/zoom)
         //      When setting higher scale, it's more intuitive for the world to become bigger (= camera become smaller),
         //      not for the camera getting bigger, hence the invert. Same deal with rotation.
         //   3. Move it by (-offset);
         //      Offset defines target transform relative to screen, but since we're effectively "moving" screen (camera)
         //      we need to do it into opposite direction (inverse transform)

         // Having camera transform in world-space, inverse of it gives the modelview transform.
         // Since (A*B*C)' = C'*B'*A', the modelview is
         //   1. Move to offset
         //   2. Rotate and Scale
         //   3. Move by -target
         auto target = getTarget();
         Matrix matOrigin = MatrixTranslate(-target.x, -target.y, 0.0f);
         Matrix matRotation = MatrixRotate((Vector3){ 0.0f, 0.0f, 1.0f }, transform.rotation);
         Matrix matScale = MatrixScale(transform.scale.x, transform.scale.y, 1.0f);
         Matrix matTranslation = MatrixTranslate(transform.translation.x, transform.translation.y, 0.0f);

         matTransform = MatrixMultiply(MatrixMultiply(matOrigin, MatrixMultiply(matScale, matRotation)), matTranslation);

         return matTransform;
      }
   protected:
      Camera2D(const std::string& instanceName, const Size<int>& screenSize)
      : REYENGINE_CTOR_INIT_LIST(instanceName, BaseWidget)
//      , PROPERTY_DECLARE(_cameraTransform)
      , active(PROP_ACTIVE_NAME, true)
      {
         _isCamera = true;
//         _camera.camera.offset = (Vector2)(screenSize/2);
         applyRect({{0, 0},screenSize});
      }
      void renderChain(Pos<R_FLOAT>& parentOffset);
      void renderBegin(ReyEngine::Pos<R_FLOAT>& textureOffset) override;
      void renderEnd() override;
      void render() const override {};
      void _on_enter_tree() override{
         //look at center of screen
         auto canvasOpt = getCanvas();
         if (!canvasOpt){
            throw std::runtime_error("Camera " + getName() + " does not have a canvas to render to! That's not good! I'm curious how you managed to do that.");
         }
         auto& canvas = canvasOpt.value();
         auto me = BaseWidget::toType<ReyEngine::Camera2D>();
         canvas->setActiveCamera(me);
         _on_rect_changed();
      };
      void _on_rect_changed() override {
         // Target is what the camera is zooming/rotating around
//         _camera.camera.target.x= getRect().x + _camera.camera.offset.x;
//         _camera.camera.target.y= getRect().y + _camera.camera.offset.y;
         //update the transform
         transform.translation = -getRect().pos();
      }
      void _on_about_to_exit_tree() override{
         ////center of screen
         auto canvasOpt = getCanvas();
         if (!canvasOpt){
            throw std::runtime_error("Camera " + getName() + " does not have a canvas to render to! That's not good! I'm curious how you managed to do that.");
         }
         auto& canvas = canvasOpt.value();
      }
      std::optional<std::shared_ptr<BaseWidget>> askHover(const ReyEngine::Pos<R_FLOAT>& globalPos) override;
      Handled __process_unhandled_input(const InputEvent&, const std::optional<UnhandledMouseInput>&) final;
      BoolProperty active;
//      Transform2DProperty _cameraTransform; //the transform that is used to scale the background - distinct from the camera's widget transform.
   private:
      static constexpr char PROP_ACTIVE_NAME[] = "active";
//      CameraStack2D _camera;
      friend class Canvas;
   };
}