#include "ModelBody.h"

using namespace ReyEngine;
using namespace std;

/////////////////////////////////////////////////////////////////////////////////////////
void ModelBody::setModel(std::shared_ptr<Model3D>& model) {
    _model = model;
}

/////////////////////////////////////////////////////////////////////////////////////////
void ModelBody::setTexture(std::shared_ptr<ReyTexture>& tex) {
    _texture = tex;
    if (_model){
        auto& model = *_model;
        model.getModel().materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = _texture->getTexture();
    }
}