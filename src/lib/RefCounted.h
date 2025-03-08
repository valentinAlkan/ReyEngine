#pragma once
#include <atomic>
#include <type_traits>
#include <utility>

namespace ReyEngine::Internal::Tree{
   template<typename T> class RefCounted;
   template<typename T> class WeakRef;
   using RefCountSizeType = uint32_t;
   class ControlBlock {
   public:
      virtual ~ControlBlock() = default;
      virtual void addStrongRef() noexcept = 0;
      virtual void releaseStrongRef() noexcept = 0;
      virtual void addWeakRef() noexcept = 0;
      virtual void releaseWeakRef() noexcept = 0;
      virtual bool tryAddStrongRef() noexcept = 0;
      virtual RefCountSizeType useCount() const noexcept = 0;
      virtual void* get_ptr() const = 0;
   };

   template<typename T>
   class TypedControlBlock : public ControlBlock {
   public:
      void talk(){std::cout << "SCount =  " << strongCount_ << ", WCount = " << weakCount_ << std::endl;}
      explicit TypedControlBlock(T* ptr) noexcept
      : ptr_(ptr), strongCount_(1), weakCount_(0) {
         talk();
      }

      void addStrongRef() noexcept override {
         auto current = strongCount_.load(std::memory_order_relaxed);
         while (current != 0 && !strongCount_.compare_exchange_weak(current, current + 1, std::memory_order_relaxed, std::memory_order_relaxed)) {/* Retry */}
         talk();
      }

      void releaseStrongRef() noexcept override {
         if (strongCount_.fetch_sub(1, std::memory_order_acq_rel) == 1) {
            ptr_.release();
            if (weakCount_.load(std::memory_order_relaxed) == 0) {
               std::cout << "Strongref releasing contorl block!" << std::endl;
            } else {
               talk();
            }
         }
      }

      void addWeakRef() noexcept override {
         weakCount_.fetch_add(1, std::memory_order_relaxed);
      }

      void releaseWeakRef() noexcept override {
         if (weakCount_.fetch_sub(1, std::memory_order_acq_rel) == 1) {
            if (strongCount_.load(std::memory_order_relaxed) == 0) {
               std::cout << "Weakref releasing contorl block!" << std::endl;
               delete this;
            }
         }
      }

      bool tryAddStrongRef() noexcept override {
         auto current = strongCount_.load(std::memory_order_relaxed);
         while (current != 0) {
            if (strongCount_.compare_exchange_weak(current, current + 1, std::memory_order_relaxed, std::memory_order_relaxed)) {
               talk();
               return true;
            }
         }
         talk();
         return false;
      }

      [[nodiscard]] RefCountSizeType useCount() const noexcept override {
         return strongCount_.load(std::memory_order_relaxed);
      }

      [[nodiscard]] void* get_ptr() const override { return (void*)ptr_.get(); }

   private:
      std::unique_ptr<T> ptr_;
      std::atomic<RefCountSizeType> strongCount_;
      std::atomic<RefCountSizeType> weakCount_;
   };

// Updated RefCounted to use ControlBlock
   template<typename T>
   class RefCounted {
   public:
      RefCounted(std::nullptr_t) noexcept : control_(nullptr) {}
      explicit RefCounted(T* ptr = nullptr) noexcept
      : control_(ptr ? new TypedControlBlock<T>(ptr) : nullptr) {}

      // Constructor from existing control block (used by WeakRef::lock())
      explicit RefCounted(ControlBlock* control) noexcept
      : control_(control) {

      }

      //other-type-aware
      template<typename U>
      RefCounted(ControlBlock* control) noexcept
      : control_(control) {
         if (control_) control_->addStrongRef();
      }

      // Constructor to convert from unique ptr
      explicit RefCounted(std::unique_ptr<T>&& data) noexcept
      : RefCounted(data.release()){}

      // Add converting constructor for inheritance
      template<typename U>
      RefCounted(RefCounted<U>&& other) noexcept
      : control_(other.control_)
      {
         other.control_ = nullptr;
      }

      template<typename U>
      RefCounted(const RefCounted<U>& other) noexcept
      : control_(other.control_)
      {
         if (control_) control_->addStrongRef();
      }

      // makes no sense in a shared ownership model. Just copy and drop.
      RefCounted& operator=(RefCounted&& other) = delete;
      RefCounted(RefCounted&& other)
      : control_(other.control_)
      {
         other.release();
      }

      RefCounted(const RefCounted& other) : control_(other.control_) {
         if (control_) control_->addStrongRef();
      }

      RefCounted& operator=(const RefCounted& other){
         if (control_) control_->addStrongRef();
         control_ = other.control_;
      }

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

      T* get() const noexcept {return control_ ? static_cast<T*>(control_->get_ptr()) : nullptr;}
      T* operator->() const noexcept { return get(); }
      T& operator*() const noexcept { return *get(); }
      [[nodiscard]] RefCountSizeType use_count() const noexcept {return control_ ? control_->useCount() : 0;}
      explicit operator bool() const noexcept {return control_;}
      ControlBlock* get_ctrl_block(){return control_;} //dangerous, make sure you know what you're doing

      WeakRef<T> getWeakRef() noexcept;

   private:
      ControlBlock* control_;
      friend class WeakRef<T>;
      template<typename U>
      friend class RefCounted;
   };

   // WeakRef implementation
   template<typename T>
   class WeakRef {
   public:
      WeakRef() noexcept : control_(nullptr) {}

      explicit WeakRef(ControlBlock* control) noexcept : control_(control) {
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
      ControlBlock* control_;
      friend class RefCounted<T>;
   };

   template<typename T>
   WeakRef<T> RefCounted<T>::getWeakRef() noexcept {return WeakRef<T>(control_);}

   // Helper function for creating RefCounted objects
   template<typename T, typename... Args>
   RefCounted<T> make_ref_counted(Args&&... args) {return RefCounted<T>(new T(std::forward<Args>(args)...));}
}