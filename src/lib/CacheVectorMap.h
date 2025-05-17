#pragma once
#include <vector>
#include <unordered_set>

template <typename T>
struct OrderedCache {
   bool add(const T& key){
      //check if it already exists in the set
      auto found = _set.find(key);
      if (found != _set.end()){
         //key found. bail out
         return false;
      }
      //otherwise emplace in set and add to vector
      _set.emplace(key);
      _vec.push_back(key);
      return true;
   }
   void remove(const T& key){
      //check if it already exists in the set
      {
         auto found = _set.find(key);
         if (found == _set.end()) return;
         //otherwise remove
         _set.erase(found);
      }
      auto found = std::find(_vec.begin(), _vec.end(), key);
      _vec.erase(found);
   }
   const std::vector<T>& getValues(){return _vec;}


//   //iterator that just wraps vector iterator
//   class iterator {
//   public:
//      // Iterator type definitions
//      using iterator_category = std::random_access_iterator_tag;
//      using value_type = T;
//      using difference_type = std::ptrdiff_t;
//      using pointer = const T*;
//      using reference = const T&;
//
//   private:
//      typename std::vector<T>::const_iterator _it;
//
//   public:
//      // Constructor
//      iterator(typename std::vector<T>::const_iterator it) : _it(it) {}
//
//      // Dereference operator
//      reference operator*() const { return *_it; }
//      pointer operator->() const { return &(*_it); }
//
//      // Increment/decrement operators
//      iterator& operator++() { ++_it; return *this; }
//      iterator operator++(int) { auto tmp = *this; ++_it; return tmp; }
//      iterator& operator--() { --_it; return *this; }
//      iterator operator--(int) { auto tmp = *this; --_it; return tmp; }
//
//      // Arithmetic operators
//      iterator operator+(difference_type n) const { return iterator(_it + n); }
//      iterator operator-(difference_type n) const { return iterator(_it - n); }
//      iterator& operator+=(difference_type n) { _it += n; return *this; }
//      iterator& operator-=(difference_type n) { _it -= n; return *this; }
//
//      // Difference between iterators
//      difference_type operator-(const iterator& other) const { return _it - other._it; }
//
//      // Comparison operators
//      bool operator==(const iterator& other) const { return _it == other._it; }
//      bool operator!=(const iterator& other) const { return _it != other._it; }
//      bool operator<(const iterator& other) const { return _it < other._it; }
//      bool operator>(const iterator& other) const { return _it > other._it; }
//      bool operator<=(const iterator& other) const { return _it <= other._it; }
//      bool operator>=(const iterator& other) const { return _it >= other._it; }
//
//      // Random access
//      reference operator[](difference_type n) const { return _it[n]; }
//   };
//
//   // Begin and end methods
//   iterator begin() const { return iterator(_vec.begin()); }
//   iterator end() const { return iterator(_vec.end()); }



private:
   std::vector<T> _vec;
   std::unordered_set<T> _set;
};