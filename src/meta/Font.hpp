#include "DrawInterface.h"
#include "Property.h"

namespace Style{
   /////////////////////////////////////////////////////////////////////////////////////////

   struct FontProperty : public Property<ReyEngine::ReyEngineFont>{
      FontProperty(const std::string& instanceName)
      : Property<ReyEngine::ReyEngineFont>(instanceName, PropertyTypes::Font, ReyEngine::ReyEngineFont())
      {}
      std::string toString() const override {
         //serialize
         return "";
      }
      ReyEngine::ReyEngineFont fromString(const std::string& str) override {
         //deserialize
         return {};
      }
   };
}