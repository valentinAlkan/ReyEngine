#pragma once
#include "BaseBody.h"
#include "DrawInterface3D.h"
#include "Viewport.h"

namespace ReyEngine {
   //Each canvas can have excatly 1 active camera at a time!!!!
   class Camera3D : public BaseBody {
      REYENGINE_OBJECT_BUILD_ONLY(Camera3D, BaseBody), PROPERTY_DECLARE(active, true){}
      REYENGINE_DEFAULT_BUILD(ReyEngine::Camera3D)
   public:
      using BaseBody::getChildren;
      void setRotation(double newRot);
      double getRotation();
      void setZoom(double newZoom);
      double getZoom();
      Pos3<double> nearToFar(const Pos3<double>&);
      Pos3<double> farToNear(const Pos3<double>&);
   protected:
      void renderer3DBegin() override;
      void renderer3DEnd() override;
      void renderer3DChain() override;
      void _on_enter_tree() override{
         //look at 0,0,0
         _camera.camera.position.x = 10;
         _camera.camera.position.y = 10;
         _camera.camera.position.z = 10;
         _camera.camera.target.x = 0;
         _camera.camera.target.y = 0;
         _camera.camera.target.z = 0;
         _camera.camera.up = {0,1,0};
         auto optViewport = getViewport();
         if (optViewport) optViewport.value()->setActiveCamera(_camera);
      };
      void _on_about_to_exit_tree() override{
         ////center of screen
         auto optViewport = getViewport();
         if (!optViewport){
            throw std::runtime_error("Camera " + getName() + " does not have a canvas to render to! That's not good! I'm curious how you managed to do that.");
         }
         auto& viewport = optViewport.value();
         viewport->deleteActiveCamera();
      }
      BoolProperty active;
   private:
      CameraTransform3D _camera;
   };
}