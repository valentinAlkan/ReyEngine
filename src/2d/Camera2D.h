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
      void setRotation(double newRot){_camera.camera.rotation = newRot;}
      double getRotation(){return _camera.camera.rotation;}
      void setZoom(double newZoom){_camera.camera.zoom = newZoom;}
      double getZoom(){return _camera.camera.zoom;}
      ///
      void setTarget(const Pos<double>& target){ setPos(target - getOffset());}
      Pos<int> getTarget() const {return getOffset() - getPos();}
      void setOffset(const Pos<double>& offset){_camera.camera.offset = (Vector2)offset;}
      Pos<double> getOffset() const {return {_camera.camera.offset.x, _camera.camera.offset.y};}
      Pos<double> nearToFar(const Pos<float>&){return {};}
      Pos<double> farToNear(const Pos<float>&){return {};}
      static std::shared_ptr<Camera2D> build(const std::string& name, const Size<int>& screenSize) {
         return Camera2D::_reyengine_make_shared(name, screenSize);
      }
   protected:
      Camera2D(const std::string& instanceName, const Size<int>& screenSize)
      : REYENGINE_CTOR_INIT_LIST(instanceName, BaseWidget)
      , active(PROP_ACTIVE_NAME, true)
      {
         _camera.camera.offset = (Vector2)(screenSize/2);
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
         _camera.camera.zoom = 1.0f;
         canvas->setActiveCamera(_camera);
         _on_rect_changed();
      };
      void _on_rect_changed() override {
         // Target is what the camera is zooming/rotating around
         _camera.camera.target.x = _rect.value.x + _camera.camera.offset.x;
         _camera.camera.target.y = _rect.value.y + _camera.camera.offset.y;
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