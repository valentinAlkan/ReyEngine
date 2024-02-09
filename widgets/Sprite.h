#pragma

#pragma once
#include "BaseWidget.h"
#include "Application.h"

class Sprite : public BaseWidget {

public:
REYENGINE_OBJECT(Sprite, BaseWidget){}
public:
   void render() const override;
   void registerProperties() override;
   void _init() override;
   void setTexture(ReyEngine::FileSystem::File);

protected:
   ReyEngine::FileSystem::File texPath;
   std::unique_ptr<ReyEngine::ReyTexture> texture;
};