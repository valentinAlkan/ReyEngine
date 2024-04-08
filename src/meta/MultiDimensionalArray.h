#pragma once
template <class T, unsigned ... RestD> struct MultiDimensionalArray;
template<class T, unsigned PrimaryD >
struct MultiDimensionalArray<T, PrimaryD>
{
   typedef T type[PrimaryD];
   type data;
   T& operator[](unsigned i) { return data[i]; }
};

template<class T, unsigned PrimaryD, unsigned ... RestD >
struct MultiDimensionalArray<T, PrimaryD, RestD...>
{
   typedef typename MultiDimensionalArray<T, RestD...>::type OneDimensionDownArrayT;
   typedef OneDimensionDownArrayT type[PrimaryD];
   type data;
   OneDimensionDownArrayT& operator[](unsigned i) { return data[i]; }
};