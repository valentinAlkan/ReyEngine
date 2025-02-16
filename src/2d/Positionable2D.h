#pragma once
#include "InputManager.h"
#include "ReyEngine.h"
#include "Property.h"

namespace ReyEngine{
   namespace Collision{
      class CollisionRect;
   }

   class Canvas;
   template <typename T>
   class Positionable2D {
   public:
      Positionable2D(Size<R_FLOAT>& size, Transform2D& targetTransform) //don't make this const, want to reject temporaries
      : size(size)
      , transform(targetTransform)
      {}
      Pos<T> getPos() const {return transform.position;}
      Pos<T> getCenter() const {return transform.position + size / 2;}
      CanvasSpace<Pos<T>> getGlobalPos() const {
         //sum up all our ancestors' positions and add our own to it
         auto sum = getPos();
         if (parent && parent != (Positionable2D<T>*)parentCanvas){ //todo: Race conditions?
            sum += parent->getGlobalPos().get();
         }
         return {sum};
      }
      [[nodiscard]] CanvasSpace<Pos<int>> getCanvasInputPos() const {
         //sum up all our ancestors' positions and add our own to it
         auto sum = InputManager::getMousePos().get() - getInputOffset();
         auto _parent = parent;
         while (_parent){
            if (_parent == (Positionable2D<T>*)parentCanvas) break;
            sum -= _parent->getInputOffset();
            _parent = _parent->parent;
         }
         return {sum};
      }
      Pos<T> canvasToLocal(const CanvasSpace<Pos <T>>& canvas) const {
         auto globalPos = getGlobalPos().get();
         auto retval = canvas.get() - globalPos;
      }
      Rect<T> canvasToLocal(const CanvasSpace<Rect<T>>& canvas) const{
         auto globalPos = getGlobalPos();
         auto retval = canvas - globalPos;
         return retval;
      }
      CanvasSpace<Pos<T>> localToCanvas(const Pos<T>& local) const {
         return local + getGlobalPos().get();
      }
      CanvasSpace<Rect<T>> localToCanvas(const Rect<T>& local) const{
         return local + getGlobalPos().get();
      }
      Positionable2D* getAt(const Pos<float>& localPos) const {
         NOT_IMPLEMENTED;
         return nullptr;
      }
      void setGlobalPos(const Vec2<R_FLOAT>& newPos){
         auto newLocalPos = canvasToLocal(newPos);
         setPos(newLocalPos);
      }
      void setInputOffset(const ReyEngine::Pos<R_FLOAT>& newOffset){_inputOffset = newOffset; _hasInputOffset = (bool)_inputOffset;}
      bool hasInputOffset(){return _hasInputOffset;}
      [[nodiscard]] ReyEngine::Pos<R_FLOAT> getInputOffset() const {return _inputOffset;}
      CanvasSpace<ReyEngine::Rect<T>> getGlobalRect() const {
         auto globalPos = getGlobalPos().get();
         CanvasSpace<ReyEngine::Rect<T>> retval(ReyEngine::Rect<T>(globalPos.x, globalPos.y, getSize().x, getSize().y));
         return retval;
      }
      ReyEngine::Rect<T> getRect() const {return {transform.position, size};}
      ReyEngine::Size<T> getSize() const {return size;}
      Transform2D& getTransform(){return transform;}
      Matrix getTransformationMatrix() const {
         //Rotate around the position, do not rotate around {0,0}
         //column ordered translation
         Matrix f = MatrixIdentity();
         f = MatrixMultiply(f, MatrixScale(transform.scale.x, transform.scale.y, 1));
         f = MatrixMultiply(f, MatrixRotate({0,0,1}, transform.rotation));
         f = MatrixMultiply(f, MatrixTranslate(transform.position.x, transform.position.y, 0));
         return f;
      }
      CanvasSpace<Matrix> getGlobalTransformationMatrix() const {
         auto f = getTransformationMatrix();
         if (parent){
            f = MatrixMultiply(f, parent->getGlobalTransformationMatrix().get());
         }
         return f;
      }
      Degrees getRotation(){return Radians(transform.rotation);}
      void setRotation(Degrees newRotation){transform.rotation = Radians(newRotation).get();}
      T getWidth() const {return size.x;}
      T getHeight() const {return size.y;}
      Vec2<T> getHeightRange() const {return {0, size.y};}
      Vec2<T> getWidthtRange() const {return {0, size.x};}
      template <typename C>
      std::shared_ptr<C> createCollider(const std::string& instanceName){
         return C::template make_collider<C>(instanceName, *this);
      }
//      std::shared_ptr<Collision::CollisionRect> make_collider(const std::string& instanceName);
      Size<R_FLOAT>& size;
      Transform2D& transform;
   protected:
      /// Bypasses normal mechanisms and just directly sets a rect
      void applySize(const Size<R_FLOAT>& newSize){size = newSize;}
      void applyRect(const Rect<R_FLOAT>& rect){
         transform.position = rect.pos();
         size = rect.size();
      }
      void setParent(Positionable2D* newParent){parent=newParent;}
   private:
      ReyEngine::Pos<float> _inputOffset; //some positionables will need to transform the input, for example if they are drawn somewhere differently than where they actually are.
      bool _hasInputOffset = false;
      Positionable2D* parent = nullptr; //potentially invalid if orphaned - non-owning
      Canvas* parentCanvas = nullptr; //the canvas that owns this widget
   };


}