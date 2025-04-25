#pragma once
#include "Widget.h"
#include "Application.h"

namespace ReyEngine{
   template <typename T>
   class BaseSprite : public Widget {
   public:
      BaseSprite& operator=(const BaseSprite& other){
         region = other.region;
         _texture = other._texture;
         texPath = other.texPath;
         _fitNextTexture = other._fitNextTexture;
         _drawDebugRect = other._drawDebugRect;
      }
      void setRegion(const T& newRegion){region = newRegion;}
      T getRegion(){return region;}
      bool setTexture(const char* filePath){
         ReyEngine::FileSystem::File f(filePath);
         return setTexture(f);
      };

      inline std::optional<const std::shared_ptr<ReyTexture>> getTexture(){
         if (_texture){
            return _texture;
         }
         return std::nullopt;
      }
      void fitTexture() { //makes the sprite the same size as the texture REGION (not necessarily the texture SIZE, in the case of a sprite sheet!)
         if (_texture){
            applySize(region.value.size());
         } else {
            _fitNextTexture = true;
         }
      }
   protected:
      BaseSprite(const std::string& texPath, const T& region)
      : texPath(texPath)
      , region(region)
      {}
      bool _drawDebugRect = true; //true if we want to see what the box looks like
      bool _fitNextTexture = false; //if we dont' yet have a texture (because window isn't loaded), then fit the texture when we do have one
      T region;
      FileSystem::File texPath;
      std::shared_ptr<ReyTexture> _texture;
      //determines if the region set for the sprite is valid (inside the texture and non-null).
      // For animated sprites, also determines if there is a region at all.
      bool isValidRegion(){
         if constexpr (std::is_same_v<T, Rect<R_FLOAT>>){
            if (!region.size()) return false; //has no width or height
            return _texture->size.toRect().contains(region);
         }
         if constexpr (std::is_same_v<T, std::vector<Rect<R_FLOAT>>>){
            if (region.empty()) return false;
            for (const auto& rect : region){
               if (!rect.size()) return false; //has no width or height
               if (!_texture->size.toRect().contains(rect)) return false;
            }
         }
         return true;
      }
   };

   class Sprite : public BaseSprite<Rect<R_FLOAT>> {
   public:
      REYENGINE_OBJECT(Sprite)
      Sprite(const FileSystem::File& texPath, const Rect<double>& region): BaseSprite<Rect<float>>(texPath, region){}
      void render2D() const override;
      void _init() override;
      bool setTexture(FileSystem::File& path); //no temporaries!
      bool setTexture(const std::shared_ptr<ReyTexture>& other);
   };


   class AnimatedSprite : public BaseSprite<std::vector<Rect<R_FLOAT>>> {
   using FrameIndex = size_t;
   public:
      REYENGINE_OBJECT(AnimatedSprite)
      AnimatedSprite(const FileSystem::File& texPath, const std::vector<Rect<R_FLOAT>>& regions): BaseSprite(texPath, regions){}
      AnimatedSprite(const FileSystem::File& texPath, Size<R_FLOAT> spriteSize, FrameIndex frameStart, FrameIndex frameCount);
      void render2D() const override;
      void setFrameIndex(FrameIndex newIndex){frameIndex = newIndex >= region.size() ? 0 : newIndex;}
      FrameIndex getFrameIndex(){return frameIndex >= region.size() ? 0 : frameIndex;}
   protected:
      FrameIndex frameIndex = 0;
   };
}