#include "DrawInterface.h"
#include "Property.h"

namespace Style{
   /////////////////////////////////////////////////////////////////////////////////////////

   struct FontProperty : public Property<GFCSDraw::GFCSDrawFont>{
      FontProperty(const std::string& instanceName)
      : Property<GFCSDraw::GFCSDrawFont>(instanceName, PropertyTypes::Font, GFCSDraw::GFCSDrawFont())
      {}
      std::string toString() override {
         //serialize
         return "";
      }
      GFCSDraw::GFCSDrawFont fromString(const std::string& str) override {
         //deserialize
         return GFCSDraw::GFCSDrawFont();
      }
   };
}