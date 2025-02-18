#include <memory>
#include <string>
#include <unordered_map>
#include <variant>
#include <stdexcept>
#include <vector>

// Base class for objects that can have properties
class MetaDataOwner {
public:
   //uses built-in copy functions to avoid alignment issues
   template <typename T>
   void setMetadata(const std::string& name, const T& value) {
      auto& storage = metadata.try_emplace(name).first->second;
      storage.resize(sizeof(T));
      auto& _reinterp = reinterpret_cast<T&>(*storage._data());
      _reinterp = value;
   }

   template<typename T>
   T& getMetadata(const std::string& name) {
      auto it = metadata.find(name);
      if (it == metadata.end()) {
         throw std::runtime_error("Property not found: " + name);
      }
      return *reinterpret_cast<T*>(it->second._data());
   }

   inline std::optional<char*> hasMetadata (const std::string& name) {
      auto it = metadata.find(name);
      if (it != metadata.end()) return it->second._data();
   }

   template<typename T>
   T& getMetadata(const std::string& name) const {return const_cast<MetaDataOwner*>(this)->getMetadata<T>(name);}
   inline std::optional<const char*> hasMetadata (const std::string& name) const {return const_cast<MetaDataOwner*>(this)->hasMetadata(name);}


protected:
   std::unordered_map<std::string, std::vector<char>> metadata;
};