#pragma once
#include <variant>
#include "Widget.h"

namespace ReyEngine {
   class TextureRect;
   class RenderTargetRect;
   namespace Internal{
      template<typename T> concept IsTextureRect = std::is_same_v<T, TextureRect>;
      // Genericized type that displays either a texture rect or a render target. The functionality is nearly identical
      // in either case, so we use a concept to determine which one we are at compile time, which is more performant and
      // easier to maintain compared with having two large blocks of almost-the-same code in two different places
      template <typename T>
      class DrawArea : public Widget{
      public:
         enum class FitType{FIT_RECT, FIT_HEIGHT, FIT_WIDTH, NONE};
         REYENGINE_OBJECT(TextureRect)
         DrawArea(FitType fit=DEFAULT_FIT): _fitType(fit){}
         void fitTexture();
         void setFitType(FitType fitType){_fitType = fitType;}
      protected:
         void render2D() const override{
            bool canFit = false;
            Size<R_FLOAT> texSize;
            if constexpr (IsTextureRect<T>){
               canFit = (bool)_texture;
               if (canFit) texSize = _texture->size;
            } else if (_renderTarget && _renderTarget->ready()){
               canFit = true;
               texSize = _renderTarget->getSize();
            }

            if (canFit) {
               Rect<float> srcRect = {{0, 0}, texSize};
               Rect<float> dstRect = getSizeRect();
               switch(_fitType) {
                  case FitType::FIT_RECT: break;
                  case FitType::FIT_WIDTH:
                     dstRect.height = srcRect.height;
                     break;
                  case FitType::FIT_HEIGHT:
                     dstRect.width = srcRect.width;
                     break;
                  case FitType::NONE:
                     srcRect = dstRect;
                     break;
               }
               ScopeScissor scopeScissor(getGlobalTransform(), getSizeRect());
               if constexpr (IsTextureRect<T>){
                  if (_texture) drawTexture(*_texture, srcRect, dstRect, Colors::none);
               } else {
                  drawRectangle(getSizeRect(), Colors::white);
                  if (_renderTarget) drawRenderTargetRect(*_renderTarget, srcRect, dstRect, Colors::none);
               }

            } else {
               auto sizeRect = getSizeRect();
               drawRectangleLines(sizeRect, 2.0, Colors::red);
               drawLine({sizeRect.topLeft(), sizeRect.bottomRight()}, 2.0, Colors::red);
               drawLine({sizeRect.bottomLeft(), sizeRect.topRight()}, 2.0, Colors::red);
            }
         }

         //only exists for texture rects
         [[no_unique_address]] std::conditional_t<IsTextureRect<T>, std::shared_ptr<ReyTexture>, std::monostate>  _texture;
         //only exists for render targets
         [[no_unique_address]] std::conditional_t<IsTextureRect<T>, std::monostate, std::shared_ptr<RenderTarget>>  _renderTarget;
         FitType _fitType = FitType::FIT_RECT;
         bool _fitScheduled = false; //if we're not inited yet
      protected:
         static constexpr FitType DEFAULT_FIT = FitType::FIT_RECT;
      };
   }

   /// Draws a texture
   class TextureRect : public Internal::DrawArea<TextureRect> {
   public:
      REYENGINE_OBJECT(TextureRect)
      TextureRect(){
         _texture = std::make_shared<ReyTexture>();
      };
      TextureRect(const FileSystem::File& f, FitType fit=DEFAULT_FIT): Internal::DrawArea<TextureRect>(fit) { setTexture(f);}
      TextureRect(const std::shared_ptr<ReyTexture>& t, FitType fit=DEFAULT_FIT): Internal::DrawArea<TextureRect>(fit){setTexture(t);}
      void setTexture(const FileSystem::File&);
      void setTexture(const std::shared_ptr<ReyTexture>&);
      void setTexture(ReyTexture&&);
      void _init() override;
   };

   /// Draws a render target
   class RenderTargetRect : public Internal::DrawArea<RenderTargetRect> {
   public:
      REYENGINE_OBJECT(RenderTargetRect)
      RenderTargetRect(std::shared_ptr<RenderTarget>& tgt){_renderTarget = tgt;};
   };
}