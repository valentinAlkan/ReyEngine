#pragma once
#include <vector>
#include "Node3D.h"

namespace ReyEngine {
   /// Draws a set of lines kept on the GPU, all one color, a pixel wide. Sending lines a vertex at a time each frame gets
   /// slow by the hundred thousand; these go up once, and drawing them costs a draw call however many there are. Hide
   /// what's out of view by keeping sets of them in nodes of their own, and setting those invisible.
   class LineMesh3D : public Node3D {
   public:
      REYENGINE_OBJECT(LineMesh3D)
      /// the lines' ends, in local space, as x, y, z of each: two points to a line. Create it once the window's open.
      explicit LineMesh3D(const std::vector<float>& points, const ColorRGBA& color = Colors::white);
      ~LineMesh3D() override;
      LineMesh3D(const LineMesh3D&) = delete;
      LineMesh3D& operator=(const LineMesh3D&) = delete;
      /// the color to draw them in, with how opaque
      void setColor(const ColorRGBA& color){_color = color;}
      [[nodiscard]] const ColorRGBA& getColor() const {return _color;}
      /// how many lines it has
      [[nodiscard]] size_t getLineCount() const {return (size_t)_vertices / 2;}
   protected:
      void render3D(RenderContext3D&) const override;
   private:
      unsigned int _vertexArray = 0, _vertexBuffer = 0;
      int _vertices = 0;
      ColorRGBA _color;
   };
}
