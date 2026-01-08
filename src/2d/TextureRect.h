#pragma once
#include <variant>
#include "Widget.h"
#include "Shader.h"

namespace ReyEngine {
   class TextureRect;
   class RenderTargetRect;
   namespace Internal{
      template<typename T> concept IsTextureRect = std::is_same_v<T, TextureRect>;
      template<typename T> concept IsRenderTargetRect = std::is_same_v<T, RenderTargetRect>;
      // Genericized type. The functionality is nearly identical
      // in any case, so we use a concept to determine which thing we are at compile time, which is more performant and
      // easier to maintain compared with having multiple large blocks of almost-the-same code in different places
      template <typename T>
      class DrawArea : public Widget {
      public:
         enum class FitType{FIT_RECT, FIT_HEIGHT, FIT_WIDTH, NONE};
         REYENGINE_OBJECT(TextureRect)
         DrawArea(FitType fit=DEFAULT_FIT): _fitType(fit){}
         void setFitType(FitType fitType){_fitType = fitType; _calculateFit();}
         void setShader(const std::shared_ptr<ReyShader>& shader){_shader = shader;}
         void setRegion(const Rect<float>& newRegion){ _region = newRegion; _calculateFit();}
         [[nodiscard]] std::shared_ptr<ReyShader> getShader() const {return _shader;}
      protected:
         void _calculateFit(){
            _dstRect = getSizeRect();
            bool ready = false;
            if constexpr (IsTextureRect<T>){
               ready = (bool)_texture;
            } else if constexpr (IsRenderTargetRect<T>) {
               ready = _renderTarget && _renderTarget->ready();
            }
            if (ready) {
               if constexpr (IsTextureRect<T>) _srcRect = _region;
               else _srcRect = getSizeRect();
               switch (_fitType) {
                  case FitType::FIT_RECT: break; //already good to go
                  case FitType::FIT_WIDTH:
                     _dstRect.height = _region.height;
                     break;
                  case FitType::FIT_HEIGHT:
                     _dstRect.width = _region.width;
                     break;
                  case FitType::NONE:
                     _dstRect = _region.toSizeRect();
                     break;
               }
            }
         }

         void render2D() const override{
            if (_dstRect.size() != Vec2<float>(0,0)) {
               ScopeScissor scopeScissor(getGlobalTransform(), getSizeRect());
               if constexpr (IsTextureRect<T>){
                  if (_texture && *_texture) {
                     drawTexture(*_texture, _srcRect, _dstRect, Colors::none);
                  } else {
                     if (_shader) {
                        BeginShaderMode(_shader->getShader());
                        _shader->_rebindTextures();
                     }
                     drawRectangle(_dstRect, Colors::black);
                     if (_shader) {
                        EndShaderMode();
                     }
                  }
               } else if constexpr (IsRenderTargetRect<T>){
                  drawRectangle(getSizeRect(), Colors::white);
                  if (_renderTarget) drawRenderTargetRect(*_renderTarget, _srcRect, _dstRect, Colors::none);
               }
            } else {
               //draw red x to signify a problem
               auto sizeRect = getSizeRect();
               drawRectangleLines(sizeRect, 2.0, Colors::red);
               drawLine({sizeRect.topLeft(), sizeRect.bottomRight()}, 2.0, Colors::red);
               drawLine({sizeRect.bottomLeft(), sizeRect.topRight()}, 2.0, Colors::red);
            }
         }
         //these members only exist for the given types
         [[no_unique_address]] std::conditional_t<IsTextureRect<T>, std::shared_ptr<ReyTexture>, std::monostate>  _texture;
         [[no_unique_address]] std::conditional_t<IsRenderTargetRect<T>, std::shared_ptr<RenderTarget>, std::monostate>  _renderTarget;
         ReyEngine::Rect<float> _region; //user defined. the region of the texture to draw.
         ReyEngine::Rect<float> _srcRect; //not necessarily user defined.
         ReyEngine::Rect<float> _dstRect; //where we should draw it
         std::shared_ptr<ReyShader> _shader;
         FitType _fitType = FitType::FIT_RECT;
         bool _fitScheduled = false; //if we're not inited yet
         static constexpr FitType DEFAULT_FIT = FitType::FIT_RECT;
         void _on_rect_changed() override {_calculateFit();}
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
      [[nodiscard]] std::shared_ptr<ReyTexture>& getTexture(){return _texture;}
      [[nodiscard]] const std::shared_ptr<ReyTexture>& getTexture() const {return _texture;}
      void _init() override;
      template <typename... Args>
      void bindTextureToShader(Args&&... args) {_shader->bindTexture(std::forward<Args>(args)...);}
   };

   /// Draws a render target
   class RenderTargetRect : public Internal::DrawArea<RenderTargetRect> {
   public:
      REYENGINE_OBJECT(RenderTargetRect)
      RenderTargetRect(std::shared_ptr<RenderTarget>& tgt){_renderTarget = tgt;};
   };

//   class ShaderRect : public Internal::DrawArea<ShaderRect> {
//   public:
//      REYENGINE_OBJECT(ShaderRect)
//      ShaderRect(){
//         _texture = std::make_shared<ReyTexture>();
//      };
//      ShaderRect(const FileSystem::File& f, FitType fit=DEFAULT_FIT): TextureRect(f, fit) {}
//      ShaderRect(const std::shared_ptr<ReyTexture>& t, FitType fit=DEFAULT_FIT): TextureRect(t, fit){}
//      void setShader(std::shared_ptr<ReyShader>& shader){_shader = shader;}
//      [[nodiscard]] std::shared_ptr<ReyShader> getShader() const {return _shader;}
//   };
}