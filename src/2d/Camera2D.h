#pragma once
#include "BaseWidget.h"
#include "Application.h"
#include "Canvas.h"
#include "Logger.h"

namespace ReyEngine {
   class Camera2D : public BaseWidget {
      REYENGINE_OBJECT(Camera2D, BaseWidget)
      {
         _camera = {0};
      }
   public:
      void setRotation(double newRot){_camera.rotation = newRot;}
      double getRotation(){return _camera.rotation;}
      void setZoom(double newZoom){_camera.zoom = newZoom;}
      double getZoom(){return _camera.zoom;}
   protected:
      void _init() override{
         ////center of screen
         auto canvasOpt = getCanvas();
         if (!canvasOpt){
            throw std::runtime_error("Camera " + getName() + " does not have a canvas to render to! That's not good!");
         }
         auto& canvas = canvasOpt.value();
         _camera.offset = canvas->getSize()/2;
         _camera.target = canvas->getSize()/2;
         _camera.zoom = 1.0f;
      };
      void renderBegin(ReyEngine::Pos<double>& textureOffset) override{BeginMode2D(_camera);}
      void renderEnd() override{EndMode2D();}
      void render() const override {};
      void _on_rect_changed() override {
         _camera.target = getPos();
         Logger::info() << "New camera target = " << Vec2<int>(_camera.target) << std::endl;
      }

   public:
      RenderCamera::Camera2D _camera;
   };
}