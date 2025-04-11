#include "Widget.h"


namespace ReyEngine {
   class TextureRect : public Widget {
   public:
      enum class FitType{FIT_RECT, FIT_HEIGHT, FIT_WIDTH, NONE};
      REYENGINE_OBJECT(TextureRect)
      TextureRect(FitType fit=DEFAULT_FIT): _fitType(fit){}
      TextureRect(const FileSystem::File& f, FitType fit=DEFAULT_FIT): _fitType(fit){ setTexture(f);}
      TextureRect(const std::shared_ptr<ReyTexture>& t, FitType fit=DEFAULT_FIT): _fitType(fit){setTexture(t);}
      void setTexture(const FileSystem::File&);
      void setTexture(const std::shared_ptr<ReyTexture>&);
      void fitTexture();
      void setFitType(FitType fitType){_fitType = fitType;}
   protected:
      void render2D() const override;
      void _init() override;
      std::shared_ptr<ReyTexture> _texture;
      std::string _texturePath;
      FitType _fitType = FitType::FIT_RECT;
      bool _fitScheduled = false; //if we're not inited yet
   private:
      static constexpr FitType DEFAULT_FIT = FitType::FIT_RECT;
   };
}