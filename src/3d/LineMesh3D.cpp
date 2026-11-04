#include "LineMesh3D.h"
#include "raymath.h"
#include "rlgl.h"

using namespace ReyEngine;

/////////////////////////////////////////////////////////////////////////////////////////
LineMesh3D::LineMesh3D(const std::vector<float>& points, const ColorRGBA& color)
: _vertices((int)(points.size() / 3))
, _color(color)
{
   if (!_vertices) return;
   _vertexArray = rlLoadVertexArray();
   rlEnableVertexArray(_vertexArray);
   _vertexBuffer = rlLoadVertexBuffer(points.data(), (int)(points.size() * sizeof(float)), false);
   rlSetVertexAttribute(RL_DEFAULT_SHADER_ATTRIB_LOCATION_POSITION, 3, RL_FLOAT, false, 0, 0);
   rlEnableVertexAttribute(RL_DEFAULT_SHADER_ATTRIB_LOCATION_POSITION);
   rlDisableVertexArray();
}

/////////////////////////////////////////////////////////////////////////////////////////
LineMesh3D::~LineMesh3D() {
   if (!_vertexArray || !IsWindowReady()) return; //once the window's gone, so is what was on the GPU
   rlUnloadVertexArray(_vertexArray);
   rlUnloadVertexBuffer(_vertexBuffer);
}

/////////////////////////////////////////////////////////////////////////////////////////
void LineMesh3D::render3D(RenderContext3D& context) const {
   if (!_vertices || !_color.a) return;
   rlDrawRenderBatchActive(); //what's been drawn so far goes first, as it would if this were drawn through the batch

   const unsigned int shader = rlGetShaderIdDefault();
   const int* locations = rlGetShaderLocsDefault();
   rlEnableShader(shader);
   const Matrix model = MatrixMultiply(context.getTransform().matrix, rlGetMatrixTransform());
   rlSetUniformMatrix(locations[RL_SHADER_LOC_MATRIX_MVP], MatrixMultiply(MatrixMultiply(model, rlGetMatrixModelview()), rlGetMatrixProjection()));
   const float diffuse[4] = {_color.r / 255.0f, _color.g / 255.0f, _color.b / 255.0f, _color.a / 255.0f};
   rlSetUniform(locations[RL_SHADER_LOC_COLOR_DIFFUSE], diffuse, RL_SHADER_UNIFORM_VEC4, 1);
   // the buffer only has positions: the shader's texture and vertex colors have to come out white
   const float white[4] = {1, 1, 1, 1};
   rlSetVertexAttributeDefault(locations[RL_SHADER_LOC_VERTEX_COLOR], white, RL_SHADER_UNIFORM_VEC4, 4);
   rlActiveTextureSlot(0);
   rlEnableTexture(rlGetTextureIdDefault());

   rlEnableVertexArray(_vertexArray);
   rlDrawVertexArrayLines(0, _vertices);
   rlDisableVertexArray();

   rlDisableTexture();
   rlDisableShader();
}
