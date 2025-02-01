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
    std::string
   , int
   , float
   , bool
   , void*
   , std::vector<char>
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

private:
   MetaDataValue value_;
};

// Base class for objects that can have properties
class MetaDataOwner {
private:
   template<typename T, typename Variant>
   struct is_not_in_variant;
   template<typename T, typename... Types>
   struct is_not_in_variant<T, std::variant<Types...>>
   : std::bool_constant<!(std::is_same_v<T, Types> || ...)> {};
   template<typename T, typename Variant>
   static constexpr bool is_not_in_variant_v = is_not_in_variant<T, Variant>::value;
public:
   //potential shallow copy, alignment issues, etc. with non-variant types
   template <typename T>
   void setMetadata(const std::string& name, const T& value) {
      auto it = metadata.find(name);
      if (it != metadata.end()){
         //update existing value
         auto data = it->second->get<std::vector<char>>().data();
         memcpy(data, &value, sizeof(T));
      } else {
         std::vector<char> storage;
         storage.resize(sizeof(T));
         memcpy(storage.data(), &value, sizeof(T));
         metadata.emplace_hint(it, name, std::make_unique<MetaData>(std::move(storage)));
      }
   }

   template<typename T>
   T& getMetadata(const std::string& name) const {
      auto it = metadata.find(name);
      if (it == metadata.end()) {
         throw std::runtime_error("Property not found: " + name);
      }
      if constexpr (is_not_in_variant_v<T, MetaData::MetaDataValue>){
         //some other type
         return *(reinterpret_cast<T*>(it->second->get<std::vector<char>>().data()));
      } else {
         //normal metadata type
         return it->second->get<T>();
      }

   }

   bool hasMetadata(const std::string& name) const {
      return metadata.find(name) != metadata.end();
   }

protected:
   std::unordered_map<std::string, std::unique_ptr<MetaData>> metadata;
};