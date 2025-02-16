#include "BaseWidget.h"


namespace ReyEngine {
   class TextureRect : public BaseWidget {
   REYENGINE_OBJECT_BUILD_ONLY(TextureRect, BaseWidget)
   , PROPERTY_DECLARE(_texturePath)
   {}
   public:
      REYENGINE_DEFAULT_BUILD(TextureRect)
      void setTexture(const FileSystem::File&);
      void setTexture(std::shared_ptr<ReyTexture>&);
      void fitTexture();
   protected:
      void registerProperties() override{};
      void render2D() const override;
      void _init() override;
      std::shared_ptr<ReyEngine::ReyTexture> _texture;
      StringProperty _texturePath;
      bool _fitScheduled = false; //if we're not inited yet
   };
}