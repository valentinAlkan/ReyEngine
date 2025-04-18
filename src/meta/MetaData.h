#pragma once
#include <any>
#include <string>
#include <optional>
#include <map>
#include "Logger.h"

namespace ReyEngine {
   struct MetaData {
      MetaData& operator=(const MetaData&) = delete;
      MetaData(const MetaData&) = delete;

      template <typename T>
      void setMetaData(void* addr, const std::string& name, const T& newData){
         auto& ptr = metaData.try_emplace(addr).first->second;
         auto& any = ptr.try_emplace(name).first->second;
         any = newData;
      }
      
      template <typename T>
      bool hasMetaData(void* addr, const std::string& name){
         auto ptrFound = metaData.find(addr);
         if (ptrFound == metaData.end()) return {};

         auto& map = ptrFound->second;
         auto found = map.find(name);
         if (found == map.end()) return {};

         auto& any = found->second;
         if (any.type() == typeid(T)) {
            return true;
         }
         return {};
      }
      
      template <typename T>
      std::optional<T> getMetaData(void* addr, const std::string& name){
         auto ptrFound = metaData.find(addr);
         if (ptrFound == metaData.end()) return {};

         auto& map = ptrFound->second;
         auto found = map.find(name);
         if (found == map.end()) return {};

         auto& any = found->second;
         if (any.type() == typeid(T)) {
            return std::any_cast<T&>(any);
         }
         Logger::error() << "getMetaData cannot return data from conflicting types";
         return {};
      }
      void removeMetaData(void* addr, const std::string& name) {
         auto ptrFound = metaData.find(addr);
         if (ptrFound == metaData.end()) return;

         auto& map = ptrFound->second;
         auto found = map.find(name);
         if (found == map.end()) return;

         map.erase(found);

         // If the map for this address is now empty, remove it from metaData
         if (map.empty()) metaData.erase(ptrFound);
      }

      void removeAllMetaData(void* addr) {
         auto ptrFound = metaData.find(addr);
         if (ptrFound == metaData.end()) return;
         metaData.erase(ptrFound);
      }
      static MetaData& instance();
   private:      
      MetaData() = default;
      std::map<void*, std::map<std::string, std::any>> metaData;
      static std::unique_ptr<MetaData> _self;
   };

   struct MetaDataInterface{
      virtual ~MetaDataInterface(){removeAllMetaData();}
      template <typename T>
      void setMetaData(const std::string& name, const T& newData){
         MetaData::instance().setMetaData(base_addr(), name, newData);
      }
      template <typename T>
      std::optional<T> hasMetaData(const std::string& name) const {
         MetaData::instance().hasMetaData<T>(base_addr(), name);
      }
      template <typename T>
      std::optional<T> getMetaData(const std::string& name) const {
         MetaData::instance().getMetaData<T>(base_addr(), name);
      }
      void removeMetaData(const std::string& name) {
         MetaData::instance().removeMetaData(base_addr(), name);
      }
      void removeAllMetaData() {
         MetaData::instance().removeAllMetaData(base_addr());
      }
   private:
      void* base_addr() const {return dynamic_cast<void*>(const_cast<MetaDataInterface*>(this));}
   };
}