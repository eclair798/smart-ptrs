#pragma once

#include "sw_fwd.h"  // Forward declaration

#include <cstddef>  // std::nullptr_t
#include <memory>

template <typename T>
class ControlBlockPtr : public ControlBlockBase {
private:
    size_t strong_ = 0;
    size_t weak_ = 0;

public:
    ControlBlockPtr(T* ptr) : ptr_(ptr) {
        ++strong_;
    }

    void IncreaseStrongCounter() override {
        ++strong_;
    }
    void DecreaseStrongCounter() override {
        --strong_;
    }

    void IncreaseWeakCounter() override {
        ++weak_;
    }
    void DecreaseWeakCounter() override {
        --weak_;
    }

    void Release() override {
        if (strong_ != 0) {
            DecreaseStrongCounter();
            if (strong_ == 0) {
                this->Delete();
                if (weak_ == 0) {
                    delete this;
                }
            }
        }
    }

    void ReleaseWeak() override {
        if (weak_ != 0) {
            DecreaseWeakCounter();
            if (weak_ == 0) {
                //                this->Delete();
                if (strong_ == 0) {
                    delete this;
                }
            }
        }
    }

    size_t UseCount() override {
        return strong_ + weak_;
    }

    size_t UseStrongCount() override {
        return strong_;
    }

    void Delete() override {
        delete ptr_;
    }

    ~ControlBlockPtr() override {
    }

private:
    T* ptr_;
};

template <typename T>
class ControlBlockObj : public ControlBlockBase {
private:
    size_t strong_ = 0;
    size_t weak_ = 0;

public:
    template <typename... Args>
    ControlBlockObj(Args&&... args) {
        new (&aligned_storage_) T(std::forward<Args>(args)...);
        ++strong_;
    }

    void IncreaseStrongCounter() override {
        ++strong_;
    }
    void DecreaseStrongCounter() override {
        --strong_;
    }

    void IncreaseWeakCounter() override {
        ++weak_;
    }
    void DecreaseWeakCounter() override {
        --weak_;
    }

    void Release() override {
        if (strong_ != 0) {
            this->DecreaseStrongCounter();
            if (strong_ == 0) {
                this->Delete();
                if (weak_ == 0) {
                    delete this;
                }
            }
        }
    }

    void ReleaseWeak() override {
        if (weak_ != 0) {
            this->DecreaseWeakCounter();
            if (weak_ == 0) {
                //                this->Delete();
                if (strong_ == 0) {
                    delete this;
                }
            }
        }
    }

    size_t UseCount() override {
        return strong_ + weak_;
    }

    size_t UseStrongCount() override {
        return strong_;
    }

    void Delete() override {
        auto ptr = GetPtr();
        if (ptr) {
            ptr->~T();
        }
    }

    ~ControlBlockObj() override {
    }

    const T* GetPtr() const {
        auto ptr = reinterpret_cast<T*>(&aligned_storage_);
        return ptr;
    }

    T* GetPtr() {
        auto ptr = reinterpret_cast<T*>(&aligned_storage_);
        return ptr;
    }

private:
    std::aligned_storage_t<sizeof(T), alignof(T)> aligned_storage_;
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

    template <typename S>
    friend class WeakPtr;

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
        IncreaseStrongCounter();
    }

    SharedPtr(SharedPtr<T>&& other)
        : block_(std::forward<ControlBlockBase*>(other.block_)),
          observed_(std::forward<T*>(other.observed_)) {
        other.block_ = nullptr;
        other.observed_ = nullptr;
    }

    template <typename Y>
    SharedPtr(const SharedPtr<Y>& other, T* ptr) : block_(other.block_), observed_(ptr) {
        IncreaseStrongCounter();
    }

    // Promote `WeakPtr`
    // #11 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr

    explicit SharedPtr(const WeakPtr<T>& other) {
        if (other.Expired()) {
            throw BadWeakPtr();
        } else {
            block_ = other.block_;
            observed_ = other.observed_;
            IncreaseStrongCounter();
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    SharedPtr& operator=(const SharedPtr<T>& other) {
        if (observed_ == other.observed_) {
            return *this;
        }
        Release();
        block_ = other.block_;
        observed_ = other.observed_;
        IncreaseStrongCounter();

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
        IncreaseStrongCounter();
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
        IncreaseStrongCounter();
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
        return block_->UseStrongCount();
    }

    void IncreaseStrongCounter() {
        if (observed_ != nullptr) {
            block_->IncreaseStrongCounter();
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
//     SharedPtr<T> SharedFromThis();
//     SharedPtr<const T> SharedFromThis() const;
//
//     WeakPtr<T> WeakFromThis() noexcept;
//     WeakPtr<const T> WeakFromThis() const noexcept;
// };
