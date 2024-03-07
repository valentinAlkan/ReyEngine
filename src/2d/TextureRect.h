#include "BaseWidget.h"


namespace ReyEngine {
   class TextureRect : public BaseWidget {
   REYENGINE_OBJECT(TextureRect, BaseWidget)
   , PROPERTY_DECLARE(_texturePath)
   {}
   public:
      void setTexture(const FileSystem::File&);
      void fitTexture();
   protected:
      void registerProperties() override{};
      void render() const override;
      void _init() override;
      ReyEngine::ReyTexture _texture;
      StringProperty _texturePath;
      bool _fitScheduled = false; //if we're not inited yet
   };
}