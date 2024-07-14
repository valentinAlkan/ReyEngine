#pragma

#pragma once
#include "BaseWidget.h"
#include "Application.h"

namespace ReyEngine{
   class Sprite : public BaseWidget {

   public:
   REYENGINE_OBJECT_BUILD_ONLY(Sprite, BaseWidget)
      , PROPERTY_DECLARE(region)
      {}
   public:
      void render() const override;
      void registerProperties() override;
      void _init() override;
      bool setTexture(const FileSystem::File&);
      std::optional<const std::reference_wrapper<ReyTexture>> getTexture();
      void setRegion(const Rect<int>&);
      void fitTexture(); //makes the sprite the same size as the texture REGION (not necessarily the texture SIZE!)

   protected:
      FileSystem::File texPath;
      std::unique_ptr<ReyTexture> texture;
      RectProperty<int> region;

   private:
      bool _fitNextTexture = false; //if we dont' yet have a texture (because window isn't loaded), then fit the texture when we do have one
   };


   class AnimatedSprite : public BaseWidget {

   public:
   REYENGINE_OBJECT_BUILD_ONLY(AnimatedSprite, BaseWidget){}
   public:
      void render() const override;
      void registerProperties() override;
      void _init() override;

   protected:
   };
}