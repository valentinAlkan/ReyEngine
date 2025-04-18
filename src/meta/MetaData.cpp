#include "MetaData.h"

using namespace ReyEngine;

std::unique_ptr<MetaData> MetaData::_self;

/////////////////////////////////////////////////////////////////////////////////////////
MetaData& MetaData::instance() {
   if (!_self){
      _self.reset(new MetaData);
   }
   return *_self;
}