#pragma once
#include "Layout.hpp"

class Editor : public VLayout {
   GFCSDRAW_OBJECT(Editor, VLayout){}
public:
   void _init() override;
};