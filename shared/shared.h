#pragma once

#include "sw_fwd.h"  // Forward declaration

#include <cstddef>  // std::nullptr_t

class ControlBlockBase {
public:
    size_t counter_ = 0;

    void IncreaseCounter() {
        ++counter_;
    }
    void DecreaseCounter() {
        --counter_;
    }

    virtual void Delete() {
    }

    virtual ~ControlBlockBase() {
    }

    void Release() {
        if (*this) {
            this->DecreaseCounter();
            if (this->counter_ == 0) {
                this->Delete();
                delete this;
            }
        }
    }

    explicit operator bool() const {
        return counter_ != 0;
    }
};

template <typename T>
class ControlBlockPtr : public ControlBlockBase {
public:
    ControlBlockPtr(T* ptr) : ptr_(ptr) {
        ControlBlockBase::IncreaseCounter();
    }

    //    ControlBlockPtr(ControlBlockPtr&& cb)
    //        : ptr_(std::forward<T*>(cb.ptr_)),
    //          ControlBlockBase::counter_(std::forward<size_t>(cb.counter_)) {
    //    }
    //
    //    ControlBlockPtr& operator=(const ControlBlockPtr& cb) {
    //        ptr_ = cb.ptr_;
    //        ControlBlockBase::counter_ = cb.counter_;
    //    }
    //
    //    ControlBlockPtr& operator=(ControlBlockPtr&& cb) {
    //    }

    T* GetPtr() {
        return ptr_;
    }

    const T* GetPtr() const {
        return ptr_;
    }

    void Delete() {
        delete ptr_;
    }

    ~ControlBlockPtr() {
        ControlBlockBase::Release();
    }

private:
    T* ptr_;
};

template <typename T>
class ControlBlockObj : public ControlBlockBase {
public:
    template <typename... Args>
    ControlBlockObj(Args&&... args) : obj_(std::forward<Args>(args)...) {
        ControlBlockBase::IncreaseCounter();
    }

    const T* GetPtr() const {
        return &obj_;
    }

    void Delete() {
    }

    ~ControlBlockObj() {
        ControlBlockBase::Release();
    }

    T* GetPtr() {
        return &obj_;
    }

private:
    T obj_;
};

// https://en.cppreference.com/w/cpp/memory/shared_ptr
template <typename T>
class SharedPtr {
private:
    ControlBlockBase* block_ = nullptr;
    T* observed_ = nullptr;

public:
    template <typename S>
    friend class SharedPtr;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    SharedPtr() {
    }

    SharedPtr(std::nullptr_t) {
    }

    SharedPtr(T* ptr) : block_(new ControlBlockPtr<T>(ptr)), observed_(ptr) {
    }

    SharedPtr(ControlBlockObj<T>* cb) : block_(cb), observed_(cb->GetPtr()) {
    }

    SharedPtr(const SharedPtr<T>& other) : block_(other.block_), observed_(other.observed_) {
        IncreaseCounter();
    }

    SharedPtr(SharedPtr<T>&& other)
        : block_(std::forward<ControlBlockBase*>(other.block_)),
          observed_(std::forward<T*>(other.observed_)) {
        other.block_ = nullptr;
        other.observed_ = nullptr;
    }

    template <typename Y>
    SharedPtr(const SharedPtr<Y>& other, T* ptr) : block_(other.block_), observed_(ptr) {
        IncreaseCounter();
    }

    //    // Promote `WeakPtr`
    //    // #11 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr
    //    explicit SharedPtr(const WeakPtr<T>& other) {
    //    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    SharedPtr& operator=(const SharedPtr<T>& other) {
        if (observed_ == other.observed_) {
            return *this;
        }
        Release();
        block_ = other.block_;
        observed_ = other.observed_;
        IncreaseCounter();

        return *this;
    }

    SharedPtr& operator=(SharedPtr<T>&& other) {
        if (observed_ == other.observed_) {
            return *this;
        }
        Release();
        block_ = std::forward<ControlBlockBase*>(other.block_);
        observed_ = std::forward<T*>(other.observed_);
        other.block_ = nullptr;
        other.observed_ = nullptr;
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors with templates

    template <typename S>
    SharedPtr(S* ptr) : block_(new ControlBlockPtr<S>(ptr)), observed_(reinterpret_cast<T*>(ptr)) {
    }

    template <typename S>
    SharedPtr(const SharedPtr<S>& other)
        : block_(other.block_), observed_(reinterpret_cast<T*>(other.observed_)) {
        IncreaseCounter();
    }

    template <typename S>
    SharedPtr(SharedPtr<S>&& other)
        : block_(std::forward<ControlBlockBase*>(other.block_)),
          observed_(std::forward<T*>(reinterpret_cast<T*>(other.observed_))) {
        other.block_ = nullptr;
        other.observed_ = nullptr;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s with templates

    template <typename S>
    SharedPtr& operator=(const SharedPtr<S>& other) {
        if (observed_ == other.observed_) {
            return *this;
        }
        Release();
        block_ = other.block_;
        observed_ = reinterpret_cast<T*>(other.observed_);
        IncreaseCounter();
        return *this;
    }

    template <typename S>
    SharedPtr& operator=(SharedPtr<S>&& other) {
        if (observed_ == other.observed_) {
            return *this;
        }
        Release();
        block_ = std::forward<ControlBlockBase*>(other.block_);
        observed_ = std::forward<T*>(reinterpret_cast<T*>(other.observed_));
        other.block_ = nullptr;
        other.observed_ = nullptr;
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    void Release() {
        if (observed_ != nullptr) {
            observed_ = nullptr;
            block_->Release();
            block_ = nullptr;
        }
    }

    ~SharedPtr() {
        Release();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    void Reset() {
        Release();
    }

    void Reset(T* ptr) {
        auto temp = block_;
        block_ = new ControlBlockPtr(ptr);
        observed_ = ptr;
        if (temp != nullptr) {
            temp->Release();
        }
    }

    template <typename S>
    void Reset(S* ptr) {
        auto temp = block_;
        block_ = new ControlBlockPtr<S>(ptr);
        observed_ = reinterpret_cast<T*>(ptr);
        if (temp != nullptr) {
            temp->Release();
        }
    }

    void Swap(SharedPtr& other) {
        std::swap(block_, other.block_);
        std::swap(observed_, other.observed_);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const {
        if (*this) {
            return observed_;
        } else {
            return nullptr;
        }
    }

    T* Get() {
        if (*this) {
            return observed_;
        } else {
            return nullptr;
        }
    }

    T& operator*() {
        return *Get();
    }

    T& operator*() const {
        return *Get();
    }

    T* operator->() {
        return Get();
    }

    T* operator->() const {
        return Get();
    }

    size_t UseCount() const {
        if (block_ == nullptr) {
            return 0;
        }
        return block_->counter_;
    }

    void IncreaseCounter() {
        if (observed_ != nullptr) {
            block_->IncreaseCounter();
        }
    }

    explicit operator bool() const {
        return (observed_ != nullptr);
    }
};

template <typename T, typename U>
inline bool operator==(const SharedPtr<T>& left, const SharedPtr<U>& right) {
    return left.Get() = right.Get();
}

// Allocate memory only once
template <typename T, typename... Args>
SharedPtr<T> MakeShared(Args&&... args) {
    return SharedPtr<T>(new ControlBlockObj<T>(std::forward<Args>(args)...));
}

// Look for usage examples in tests
// template <typename T>
// class EnableSharedFromThis {
// public:
//    SharedPtr<T> SharedFromThis();
//    SharedPtr<const T> SharedFromThis() const;
//
//    WeakPtr<T> WeakFromThis() noexcept;
//    WeakPtr<const T> WeakFromThis() const noexcept;
//};
