#pragma once
#include <map>
#include <vector>

//A map and a vector that keeps track of the order of insertion
template <typename K, typename V>
class IndexedMap {
   using KVP = std::pair<K, V>;
   using Locator = std::pair<std::size_t, KVP>;
   void insert(K key, V value, std::size_t index){
      //do a vector style insert
      _vector.insert();
   }
   void push_back(V);
   V at();
   void find(V);
//   void erase(std::map<K,V>::iterator);
private:
   //keyval pair stored in vector, map just stores a reference to the vector entry. this way order and
   std::vector<std::pair<K, V>> _vector;
   std::map<K, Locator> _map;
};