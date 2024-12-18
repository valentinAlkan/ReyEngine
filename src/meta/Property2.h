//#include <memory>
//#include <string>
//#include <unordered_map>
//#include <variant>
//#include <stdexcept>
//#include <vector>
//
//// Simplified property system that uses std::variant for type safety
//class Property {
//public:
//   using PropertyValue = std::variant<
//   std::string,
//   int,
//   float,
//   bool,
//   std::vector<int>,
//   std::vector<std::string>
//   >;
//
//   explicit Property(PropertyValue value) : value_(std::move(value)) {}
//
//   template<typename T>
//   T& get() {
//      try {
//         return std::get<T>(value_);
//      } catch(const std::bad_variant_access&) {
//         throw std::runtime_error("Invalid property type access");
//      }
//   }
//
//   template<typename T>
//   const T& get() const {
//      try {
//         return std::get<T>(value_);
//      } catch(const std::bad_variant_access&) {
//         throw std::runtime_error("Invalid property type access");
//      }
//   }
//
//   // Serialization support
//   std::string serialize() const;
//   static Property deserialize(const std::string& data);
//
//private:
//   PropertyValue value_;
//};
//
//// Base class for objects that can have properties
//class PropertyOwner {
//public:
//   template<typename T>
//   void setProperty(const std::string& name, T value) {
//      properties_[name] = std::make_unique<Property>(std::move(value));
//   }
//
//   template<typename T>
//   T& getProperty(const std::string& name) {
//      auto it = properties_.find(name);
//      if (it == properties_.end()) {
//         throw std::runtime_error("Property not found: " + name);
//      }
//      return it->second->get<T>();
//   }
//
//   bool hasProperty(const std::string& name) const {
//      return properties_.find(name) != properties_.end();
//   }
//
//protected:
//   std::unordered_map<std::string, std::unique_ptr<Property>> properties_;
//};