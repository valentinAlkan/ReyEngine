#pragma once
#include "BaseWidget.h"

namespace ReyEngine {
   class Camera2D : public BaseWidget {
      REYENGINE_OBJECT(Camera2D, BaseWidget)
      {
         _camera = {0};
      }
      void _init() override{
         _camera.target = getGlobalPos();
         _camera.zoom = 1.0f;
      };
      void render() const override {};

   private:
      RenderCamera::Camera2D _camera;
   };
}