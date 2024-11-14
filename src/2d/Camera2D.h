#pragma once
#include "BaseWidget.h"
#include "Application.h"
#include "Canvas.h"
#include "Logger.h"

namespace ReyEngine {

   //Each canvas can have excatly 1 active camera at a time!!!!
   class Camera2D : public BaseWidget {
      REYENGINE_OBJECT_BUILD_ONLY(Camera2D, BaseWidget)
      , active("active", true)
      {

      }
   public:
      REYENGINE_DEFAULT_BUILD(Camera2D)
      void setRotation(double newRot){_camera.camera.rotation = newRot;}
      double getRotation(){return _camera.camera.rotation;}
      void setZoom(double newZoom){_camera.camera.zoom = newZoom;}
      double getZoom(){return _camera.camera.zoom;}
      /// Makes the camera 'look' at a specific point by centering its view on it
      void setTarget(const Pos<double>& target){ setPos(getPos() + getOffset());}
      void setOffset(const Pos<double>& offset){_camera.camera.offset = offset;}
      Pos<double> getOffset(){return {_camera.camera.offset.x, _camera.camera.offset.y};}
      Pos<double> nearToFar(const Pos<float>&){return {};}
      Pos<double> farToNear(const Pos<float>&){return {};}
      static std::shared_ptr<Camera2D> build(const std::string& name, const Size<int>& ScreenSize) {
         return std::shared_ptr<Camera2D>(new Camera2D(name));
      }
   protected:
      Camera2D(const std::string& instanceName, const Size<int>& ScreenSize)
      : REYENGINE_CTOR_INIT_LIST(instanceName, BaseWidget)
      , active(PROP_ACTIVE_NAME, true)
      {

      }
      void renderBegin(ReyEngine::Pos<double>& textureOffset) override;
      void renderEnd() override;
      void render() const override {};
      void _on_enter_tree() override{
         //look at center of screen
         auto canvasOpt = getCanvas();
         if (!canvasOpt){
            throw std::runtime_error("Camera " + getName() + " does not have a canvas to render to! That's not good! I'm curious how you managed to do that.");
         }
         auto& canvas = canvasOpt.value();
         auto offset = canvas->getSize()/2.0;
         _camera.camera.offset.x = offset.x;
         _camera.camera.offset.y = offset.y;
         _camera.camera.zoom = 1.0f;
         canvas->setActiveCamera(_camera);
      };
      void _on_rect_changed() override {
         _camera.camera.target.x = _rect.value.x;
         _camera.camera.target.y = _rect.value.y;
      }
      void _on_about_to_exit_tree() override{
         ////center of screen
         auto canvasOpt = getCanvas();
         if (!canvasOpt){
            throw std::runtime_error("Camera " + getName() + " does not have a canvas to render to! That's not good! I'm curious how you managed to do that.");
         }
         auto& canvas = canvasOpt.value();
         canvas->deleteActiveCamera();
      }
      BoolProperty active;
   private:
      static constexpr char PROP_ACTIVE_NAME[] = "active";
      CameraTransform2D _camera;
   };
}