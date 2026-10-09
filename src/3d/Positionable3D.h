#pragma once
#include "ReyObject.h"
#include "Positionable2D.h"
#include "ReyEngine3D.h"

namespace ReyEngine::Internal {
   /// 3D counterpart of Positionable2D. Holds a local Transform3D and resolves the global one by walking up the tree.
   class Positionable3D : public virtual TypeTag {
   public:
      [[nodiscard]] inline Pos3<R_FLOAT> getPosition() const {return Pos3<R_FLOAT>(transform3D.extractTranslation());}
      [[nodiscard]] inline Quaternion getRotation() const {return transform3D.extractRotation();}
      [[nodiscard]] inline Vec3<R_FLOAT> getScale() const {return transform3D.extractScale();}

      inline void setPosition(const Pos3<R_FLOAT>& newPosition){
         if (isLocked) return;
         auto old = transform3D;
         transform3D.setPosition(newPosition);
         __on_transform_changed(old);
      }
      inline void setPosition(R_FLOAT x, R_FLOAT y, R_FLOAT z){setPosition({x, y, z});}
      inline void movePosition(const Vec3<R_FLOAT>& amt){setPosition(Pos3<R_FLOAT>(getPosition() + amt));}
      inline void setRotation(const Quaternion& newRotation){
         if (isLocked) return;
         auto old = transform3D;
         transform3D.setRotation(newRotation);
         __on_transform_changed(old);
      }
      inline void setRotation(const Vec3<R_FLOAT>& axis, const Radians& angle){setRotation(QuaternionFromAxisAngle(axis, (float)angle.get()));}
      //rotate relative to the current rotation, in local space
      inline void rotate(const Vec3<R_FLOAT>& axis, const Radians& angle){
         if (isLocked) return;
         auto old = transform3D;
         transform3D.rotate(axis, angle);
         __on_transform_changed(old);
      }
      inline void setScale(const Vec3<R_FLOAT>& newScale){
         if (isLocked) return;
         auto old = transform3D;
         transform3D.setScale(newScale);
         __on_transform_changed(old);
      }
      inline void setScale(R_FLOAT uniformScale){setScale({uniformScale, uniformScale, uniformScale});}
      inline void setTransform(const Transform3D& newTransform){
         if (isLocked) return;
         auto old = transform3D;
         transform3D = newTransform;
         __on_transform_changed(old);
      }
      inline Transform3D& getLocalTransform(){return transform3D;}
      [[nodiscard]] inline const Transform3D& getLocalTransform() const {return transform3D;}
      /// Transform from local space to the space of the owning Canvas3D (ie world space).
      /// The 3D chain ends at the first 2D ancestor, which is normally the Canvas3D itself.
      [[nodiscard]] inline Transform3D getGlobalTransform() const {
         auto retval = getLocalTransform();
         auto parent = selfNode->getParent();
         while (parent){
            if (auto isPositionable = parent->tag<Positionable3D>()){
               retval *= isPositionable.value()->transform3D;
            } else if (parent->tag<Positionable2D>()) {
               break;
            }
            parent = parent->getParent();
         }
         return retval;
      }
      [[nodiscard]] inline Pos3<R_FLOAT> getGlobalPosition() const {return Pos3<R_FLOAT>(getGlobalTransform().extractTranslation());}
      [[nodiscard]] inline Pos3<R_FLOAT> toGlobalPos(const Pos3<R_FLOAT>& local) const {return local.transform(getGlobalTransform().matrix);}
      [[nodiscard]] inline Pos3<R_FLOAT> toLocalPos(const Pos3<R_FLOAT>& global) const {return global.transform(getGlobalTransform().inverse().matrix);}

   protected:
      virtual void __on_transform_changed(const Transform3D& oldTransform){}; //internal

      Transform3D transform3D;
      bool isLocked = false;
   };
}
