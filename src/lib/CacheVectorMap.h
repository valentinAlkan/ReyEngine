//#pragma once
//#include <vector>
//#include "MapValueRefView.h"
//
//#define FWD_FX(target, fxName) template<typename... Args> auto fxName(Args&&... args){return handle_##fxName(target.fxName(std::forward<Args>(args)...));}
//#define RETURN_RETVAL return std::forward<decltype(retval)>(retval)
//namespace ReyEngine::lib {
//   template <typename KeyType, typename OwnedType>
//   struct CacheVectorMap {
//      FWD_FX(_map, try_emplace);
//      FWD_FX(_map, at);
//      FWD_FX(_map, insert);
//      FWD_FX(_map, erase);
//      inline auto handle_try_emplace(auto&& retval) {
//         return handle_emplace(std::forward<decltype(retval)>(retval)); //same handler
//      }
//      inline auto handle_emplace(auto&& retval) {
//         bool didInsert = retval.second;
//         OwnedType& ownedType = *(retval.first->second);
//         if (didInsert){
//            //new thing was added to map
//            _vector.push_back(&ownedType);
//         } else {
//            //existing thing updated ; update it
//            for (auto& value : _vector){
////               if (_map.find())
//            }
//         }
//         RETURN_RETVAL;
//      }
//      inline auto handle_at(auto&& retval) {RETURN_RETVAL;}
//      inline auto handle_insert(auto&& retval) {
//
//      }
//      inline auto handle_erase(auto&& retval) {
//
//      }
//      auto& mapValues(){return ReyEngine::lib::values_ref<decltype(_map)>(_map);}
//      const auto& mapValues() const {return ReyEngine::lib::values_ref<decltype(_map)>(_map);}
//      auto& vecValues(){return _vector;}
//      const auto& vecValues() const {return _vector;}
//      const auto& decomposeMap(){return _map;}
//      const auto& decomposeVec(){return _vector;}
//   private:
//      std::map<KeyType, std::unique_ptr<OwnedType>> _map;
//      std::vector<OwnedType*> _vector;
//   };
//}
//
//#undef FWD_FX