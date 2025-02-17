#include <atomic>
#include <type_traits>
#include <utility>

namespace ReyEngine::Internal::Tree{
   template<typename T> class RefCounted;
   template<typename T> class WeakRef;

   // Control block that will be shared between RefCounted and WeakRef
   template<typename T>
   class ControlBlock {
   public:
      explicit ControlBlock(T* ptr) noexcept
      : ptr_(ptr)
      , strongCount_(1)
      , weakCount_(0) {
         talk();
      }
      void talk(){std::cout << "Ref count is now " << strongCount_ << std::endl;}
      void addStrongRef() noexcept {
         auto current = strongCount_.load(std::memory_order_relaxed);
         while (current != 0 && !strongCount_.compare_exchange_weak(current, current + 1, std::memory_order_relaxed, std::memory_order_relaxed)) {/* Retry on failure*/}
         talk();
      }

      void releaseStrongRef() noexcept {
         if (strongCount_.fetch_sub(1, std::memory_order_acq_rel) == 1) {
            delete ptr_;
            ptr_ = nullptr;
            if (weakCount_.load(std::memory_order_relaxed) == 0) {
               delete this;
            }
         }
         talk();
      }

      void addWeakRef() noexcept {
         weakCount_.fetch_add(1, std::memory_order_relaxed);
      }

      void releaseWeakRef() noexcept {
         if (weakCount_.fetch_sub(1, std::memory_order_acq_rel) == 1) {
            if (strongCount_.load(std::memory_order_relaxed) == 0) {
               delete this;
            }
         }
      }

      bool tryAddStrongRef() noexcept {
         auto current = strongCount_.load(std::memory_order_relaxed);
         while (current != 0) {
            if (strongCount_.compare_exchange_weak(current, current + 1, std::memory_order_relaxed, std::memory_order_relaxed)) {
               talk();
               return true;
            }
         }
         return false;
      }

      T* get() const noexcept { return ptr_; }

      long useCount() const noexcept {
         return strongCount_.load(std::memory_order_relaxed);
      }

   private:
      T* ptr_;
      std::atomic<size_t> strongCount_;
      std::atomic<size_t> weakCount_;
   };

// Updated RefCounted to use ControlBlock
   template<typename T>
   class RefCounted {
   public:
      explicit RefCounted(T* ptr = nullptr) noexcept
      : control_(ptr ? new ControlBlock<T>(ptr) : nullptr) {}

      // Constructor from existing control block (used by WeakRef::lock())
      explicit RefCounted(ControlBlock<T>* control) noexcept
      : control_(control) {}

      RefCounted(RefCounted&& other) noexcept
      : control_(other.control_) {
         other.control_ = nullptr;
      }

      RefCounted& operator=(RefCounted&& other) noexcept {
         if (this != &other) {
            if (control_) {
               control_->releaseStrongRef();
            }
            control_ = other.control_;
            other.control_ = nullptr;
         }
         return *this;
      }

      RefCounted(const RefCounted&) = delete;
      RefCounted& operator=(const RefCounted&) = delete;

      ~RefCounted() {
         if (control_) {
            control_->releaseStrongRef();
         }
      }

      void addRef() noexcept {
         if (control_) {
            control_->addStrongRef();
         }
      }

      void release() noexcept {
         if (control_) {
            control_->releaseStrongRef();
         }
      }

      T* get() const noexcept {return control_ ? control_->get() : nullptr;}
      T* operator->() const noexcept { return get(); }
      T& operator*() const noexcept { return *get(); }
      [[nodiscard]] size_t use_count() const noexcept {return control_ ? control_->useCount() : 0;}
      explicit operator bool() const noexcept {return control_ && control_->get();}

      WeakRef<T> getWeakRef() noexcept;

   private:
      ControlBlock<T>* control_;
      friend class WeakRef<T>;
   };

   // WeakRef implementation
   template<typename T>
   class WeakRef {
   public:
      WeakRef() noexcept : control_(nullptr) {}

      explicit WeakRef(ControlBlock<T>* control) noexcept : control_(control) {
         if (control_) {
            control_->addWeakRef();
         }
      }

      WeakRef(const WeakRef& other) noexcept : control_(other.control_) {
         if (control_) {
            control_->addWeakRef();
         }
      }

      WeakRef(WeakRef&& other) noexcept : control_(other.control_) {
         other.control_ = nullptr;
      }

      ~WeakRef() {
         if (control_) {
            control_->releaseWeakRef();
         }
      }

      WeakRef& operator=(const WeakRef& other) noexcept {
         if (this != &other) {
            if (control_) {
               control_->releaseWeakRef();
            }
            control_ = other.control_;
            if (control_) {
               control_->addWeakRef();
            }
         }
         return *this;
      }

      WeakRef& operator=(WeakRef&& other) noexcept {
         if (this != &other) {
            if (control_) {
               control_->releaseWeakRef();
            }
            control_ = other.control_;
            other.control_ = nullptr;
         }
         return *this;
      }

      RefCounted<T> lock() const noexcept {
         if (control_ && control_->tryAddStrongRef()) {
            // Create RefCounted from existing control block
            return RefCounted<T>(control_);
         }
         //return empty refcounted
         return RefCounted<T>();
      }

      [[nodiscard]] bool expired() const noexcept {
         return !control_ || control_->useCount() == 0;
      }

      void reset() noexcept {
         if (control_) {
            control_->releaseWeakRef();
            control_ = nullptr;
         }
      }

   private:
      ControlBlock<T>* control_;
      friend class RefCounted<T>;
   };

   template<typename T>
   WeakRef<T> RefCounted<T>::getWeakRef() noexcept {return WeakRef<T>(control_);}

   // Helper function for creating RefCounted objects
   template<typename T, typename... Args>
   RefCounted<T> make_ref_counted(Args&&... args) {return RefCounted<T>(new T(std::forward<Args>(args)...));}
}