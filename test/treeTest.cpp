#include "AbstractTree.h"

using namespace std;
using namespace ReyEngine;
using namespace Internal;
using namespace Tree;

int main(){
   // Example usage:
   WeakRef<int> weak;
   {
      auto ref = make_ref_counted<int>(42);
      weak = ref.getWeakRef();
      if (auto handle = weak.lock()) {
         // Use the handle
         *handle = 43;
         assert(handle);
      }

      assert(!weak.expired());
      cout << weak.expired() << endl;
   }
   assert(weak.expired());

   /




   return 0;
}