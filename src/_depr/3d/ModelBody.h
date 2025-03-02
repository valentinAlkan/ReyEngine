#pragma once
#include "BaseBody.h"

namespace ReyEngine {
/////////////////////////////////////////////////////////////////////////////////////////
//    struct ModelProperty: public Property<ReyEngine::Model3D>{
//        using Property<ReyEngine::Model3D>::operator=;
//        ModelProperty(const std::string& instanceName, ReyEngine::Model3D&& defaultvalue)
//        : Property(instanceName, PropertyTypes::String, std::move(defaultvalue))
//        {}
//        std::string toString() const override {return "";}
//        Model3D fromString(const std::string& data) override { return *this;}
//    };


   class ModelBody : public BaseBody {
      REYENGINE_OBJECT_BUILD_ONLY(ModelBody, BaseBody, BaseBody){
          //Load a default model which does nothing
      }
   public:
      REYENGINE_DEFAULT_BUILD(ModelBody);
      void setModel(std::shared_ptr<Model3D>&);
      void setTexture(std::shared_ptr<ReyTexture>&);
   };
}