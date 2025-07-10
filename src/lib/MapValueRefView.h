#pragma once
#include <map>
//#include <ranges>
#include <memory>

namespace ReyEngine::lib{
   template<typename Map>
   class MapValueRefView {
      Map& m;
   public:
      explicit MapValueRefView(Map& map) : m(map) {}

      // Base iterator template to avoid code duplication
      template<typename MapIterator>
      struct iterator_base {
         using map_iterator = MapIterator;
         map_iterator it;

         using value_type = typename Map::mapped_type::element_type;
         using reference = typename std::conditional_t<
         std::is_const_v<typename std::remove_reference_t<decltype(*std::declval<MapIterator>())>>,
         const value_type&,
         value_type&
         >;

         iterator_base(map_iterator i) : it(i) {}
         iterator_base& operator++() { ++it; return *this; }
         iterator_base operator++(int) { auto tmp = *this; ++it; return tmp; }
         reference operator*() const { return *(it->second); }

         template<typename OtherIterator>
         bool operator==(const iterator_base<OtherIterator>& other) const {
            return it == other.it;
         }
         template<typename OtherIterator>
         bool operator!=(const iterator_base<OtherIterator>& other) const {
            return it != other.it;
         }
      };

      // Non-const iterator
      using iterator = iterator_base<typename Map::iterator>;

      // Const iterator
      using const_iterator = iterator_base<typename Map::const_iterator>;

      // Non-const iteration
      iterator begin() { return iterator(m.begin()); }
      iterator end() { return iterator(m.end()); }

      // Const iteration
      const_iterator begin() const { return const_iterator(m.begin()); }
      const_iterator end() const { return const_iterator(m.end()); }
      const_iterator cbegin() const { return const_iterator(m.begin()); }
      const_iterator cend() const { return const_iterator(m.end()); }
   };

// Helper function to create the view
   template<typename Map>
   MapValueRefView<Map> values_ref(Map& m) {
      return MapValueRefView<Map>(m);
   }

   template<typename Map>
   const MapValueRefView<Map> const_values_ref(Map& m) {
      return MapValueRefView<Map>(m);
   }
}