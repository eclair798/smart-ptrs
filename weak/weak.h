#pragma once

#include "sw_fwd.h"  // Forward declaration

// https://en.cppreference.com/w/cpp/memory/weak_ptr

template <typename T>
class WeakPtr {
private:
    ControlBlockBase* block_ = nullptr;
    T* observed_ = nullptr;

public:
    template <typename S>
    friend class SharedPtr;

    template <typename S>
    friend class WeakPtr;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    WeakPtr() {
    }

    WeakPtr(const WeakPtr<T>& other) : block_(other.block_), observed_(other.observed_) {
        IncreaseWeakCounter();
    }
    WeakPtr(WeakPtr<T>&& other)
        : block_(std::forward<ControlBlockBase*>(other.block_)),
          observed_(std::forward<T*>(other.observed_)) {
        other.block_ = nullptr;
        other.observed_ = nullptr;
    }

    // Demote `SharedPtr`
    // #2 from https://en.cppreference.com/w/cpp/memory/weak_ptr/weak_ptr
    WeakPtr(const SharedPtr<T>& other) : block_(other.block_), observed_(other.observed_) {
        IncreaseWeakCounter();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    WeakPtr& operator=(const WeakPtr<T>& other) {
        if (observed_ == other.observed_) {
            return *this;
        }
        ReleaseWeak();
        block_ = other.block_;
        observed_ = other.observed_;
        IncreaseWeakCounter();

        return *this;
    }

    WeakPtr& operator=(WeakPtr<T>&& other) {
        if (observed_ == other.observed_) {
            return *this;
        }
        ReleaseWeak();
        block_ = std::forward<ControlBlockBase*>(other.block_);
        observed_ = std::forward<T*>(other.observed_);
        other.block_ = nullptr;
        other.observed_ = nullptr;
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors with templates

    template <typename S>
    WeakPtr(const WeakPtr<S>& other)
        : block_(other.block_), observed_(reinterpret_cast<T*>(other.observed_)) {
        IncreaseWeakCounter();
    }

    template <typename S>
    WeakPtr(WeakPtr<S>&& other)
        : block_(std::forward<ControlBlockBase*>(other.block_)),
          observed_(std::forward<T*>(reinterpret_cast<T*>(other.observed_))) {
        other.block_ = nullptr;
        other.observed_ = nullptr;
    }

    // Demote `SharedPtr`
    // #2 from https://en.cppreference.com/w/cpp/memory/weak_ptr/weak_ptr

    template <typename S>
    WeakPtr(const SharedPtr<S>& other)
        : block_(other.block_), observed_(reinterpret_cast<T*>(other.observed_)) {
        IncreaseWeakCounter();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s with templates

    template <typename S>
    WeakPtr& operator=(const WeakPtr<S>& other) {
        if (observed_ == other.observed_) {
            return *this;
        }
        ReleaseWeak();
        block_ = other.block_;
        observed_ = reinterpret_cast<T*>(other.observed_);
        IncreaseWeakCounter();
        return *this;
    }

    template <typename S>
    WeakPtr& operator=(WeakPtr<S>&& other) {
        if (observed_ == other.observed_) {
            return *this;
        }
        ReleaseWeak();
        block_ = std::forward<ControlBlockBase*>(other.block_);
        observed_ = std::forward<T*>(reinterpret_cast<T*>(other.observed_));
        other.block_ = nullptr;
        other.observed_ = nullptr;
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    void ReleaseWeak() {
        if (observed_ != nullptr) {
            observed_ = nullptr;
            block_->ReleaseWeak();
            block_ = nullptr;
        }
    }

    ~WeakPtr() {
        ReleaseWeak();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    void Reset() {
        ReleaseWeak();
    }

    void Swap(WeakPtr& other) {
        std::swap(block_, other.block_);
        std::swap(observed_, other.observed_);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    size_t UseCount() const {
        if (block_ == nullptr) {
            return 0;
        }
        return block_->UseStrongCount();
    }

    void IncreaseWeakCounter() {
        if (observed_ != nullptr) {
            block_->IncreaseWeakCounter();
        }
    }

    bool Expired() const {
        if (block_ != nullptr) {
            return UseCount() == 0;
        }
        return true;
    }

    T* Get() {
        if (Expired()) {
            return nullptr;
        }
        return observed_;
    }

    SharedPtr<T> Lock() const {
        if (Expired()) {
            return SharedPtr<T>();
        }
        return SharedPtr<T>(*this);
    }
};
