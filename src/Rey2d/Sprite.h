#pragma

#pragma once
#include "BaseWidget.h"
#include "Application.h"

class Sprite : public BaseWidget {

public:
REYENGINE_OBJECT(Sprite, BaseWidget)
   , PROPERTY_DECLARE(region)
   {}
public:
   void render() const override;
   void registerProperties() override;
   void _init() override;
   bool setTexture(const ReyEngine::FileSystem::File&, const ReyEngine::Rect<int>& drawRegion);
   void fitTexture(); //makes the sprite the same size as the texture REGION (not necessarily the texture SIZE!)

protected:
   ReyEngine::FileSystem::File texPath;
   std::unique_ptr<ReyEngine::ReyTexture> texture;
   RectProperty<int> region;
};


class AnimatedSprite : public BaseWidget {

public:
REYENGINE_OBJECT(AnimatedSprite, BaseWidget){}
public:
   void render() const override;
   void registerProperties() override;
   void _init() override;

protected:
};