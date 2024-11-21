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
   static std::shared_ptr<Sprite> build(const std::string& name, const FileSystem::File& texPath, const Rect<double>& region){
      auto me = build(name);
      me->texPath = texPath;
      me->region = region;
      return me;
   }
   public:
      REYENGINE_DEFAULT_BUILD(Sprite)
      void render() const override;
      void registerProperties() override;
      void _init() override;
      bool setTexture(const FileSystem::File&);
      std::optional<const std::reference_wrapper<ReyTexture>> getTexture();
      void setRegion(const Rect<double>&);
      void fitTexture(); //makes the sprite the same size as the texture REGION (not necessarily the texture SIZE, in the case of a sprite sheet!)

   protected:
      FileSystem::File texPath;
      std::unique_ptr<ReyTexture> texture;
      RectProperty<R_FLOAT> region;

   private:
      bool _fitNextTexture = false; //if we dont' yet have a texture (because window isn't loaded), then fit the texture when we do have one
      bool _drawDebugRect = true; //true if we want to see what the box looks like
   };


   class AnimatedSprite : public BaseWidget {
      public:
         REYENGINE_OBJECT_BUILD_ONLY(AnimatedSprite, BaseWidget){}
      public:
         REYENGINE_DEFAULT_BUILD(AnimatedSprite);
         void render() const override;
         void registerProperties() override;
         void _init() override;

      protected:
   };
}