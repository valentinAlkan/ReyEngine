#include <memory>
#include <string>
#include <unordered_map>
#include <variant>
#include <stdexcept>
#include <vector>

// Simplified property system that uses std::variant for type safety
class MetaData {
public:
   using MetaDataValue = std::variant<
   std::string,
   int,
   float,
   bool,
   std::vector<int>,
   std::vector<std::string>,
   void*
   >;

   explicit MetaData(MetaDataValue value) : value_(std::move(value)) {}

   template<typename T>
   T& get() {
      try {
         return std::get<T>(value_);
      } catch(const std::bad_variant_access&) {
         throw std::runtime_error("Invalid property type access");
      }
   }

   template<typename T>
   const T& get() const {
      try {
         return std::get<T>(value_);
      } catch(const std::bad_variant_access&) {
         throw std::runtime_error("Invalid property type access");
      }
   }

//   // Serialization support
//   std::string serialize() const;
//   static MetaData deserialize(const std::string& data);

private:
   MetaDataValue value_;
};

// Base class for objects that can have properties
class MetaDataOwner {
public:
   template<typename T>
   void setMetadata(const std::string& name, T value) {
      metadata[name] = std::make_unique<MetaData>(std::move(value));
   }

   template<typename T>
   T& getMetadata(const std::string& name) {
      auto it = metadata.find(name);
      if (it == metadata.end()) {
         throw std::runtime_error("Property not found: " + name);
      }
      return it->second->get<T>();
   }

   bool hasMetadata(const std::string& name) const {
      return metadata.find(name) != metadata.end();
   }

protected:
   std::unordered_map<std::string, std::unique_ptr<MetaData>> metadata;
};