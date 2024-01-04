#pragma once

#include "compressed_pair.h"

#include <cstddef>  // std::nullptr_t

template <class T>
class Slug {
public:
    Slug() = default;

    template <class S>
    Slug(const Slug<S>&) {
    }

    template <class S>
    Slug(Slug<S>&&) noexcept {
    }

    template <class S>
    Slug<T>& operator=(Slug<S>&&) noexcept {
        return *this;
    }

    ~Slug() = default;

    void operator()(T* p) const {
        delete p;
    }
};

template <class T>
class Slug<T[]> {
public:
public:
    Slug() = default;

    template <class S>
    Slug(const Slug<S[]>&) {
    }

    template <class S>
    Slug(Slug<S[]>&&) noexcept {
    }

    template <class S>
    Slug<T[]>& operator=(Slug<S[]>&&) noexcept {
        return *this;
    }

    ~Slug() = default;

    void operator()(T* p) const {
        delete[] p;
    }
};

// Primary template
template <typename T, typename Deleter = Slug<T>>
class UniquePtr {
private:
    CompressedPair<T*, Deleter> ptr_;

public:
    void Clear() {
        ptr_.GetFirst() = nullptr;
        ptr_.GetSecond() = Deleter();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    explicit UniquePtr(T* ptr = nullptr) : ptr_(ptr, Deleter()) {
    }

    UniquePtr(T* ptr, const Deleter& deleter) : ptr_(ptr, deleter) {
    }

    UniquePtr(T* ptr, Deleter&& deleter) : ptr_(ptr, std::forward<Deleter>(deleter)) {
    }

    UniquePtr(UniquePtr&& other) noexcept
        : ptr_(std::forward<CompressedPair<T*, Deleter>>(other.ptr_)) {
        other.Clear();
    }

    UniquePtr(const UniquePtr& other) = delete;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    UniquePtr& operator=(UniquePtr&& other) noexcept {
        if (other.ptr_.GetFirst() == ptr_.GetFirst()) {
            return *this;
        }
        Destructor();
        ptr_ = std::forward<CompressedPair<T*, Deleter>>(other.ptr_);
        other.Clear();
        return *this;
    }
    UniquePtr& operator=(std::nullptr_t) {
        Destructor();
        return *this;
    }

    UniquePtr& operator=(const UniquePtr& other) = delete;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors with Templates

    template <typename S, typename D>
    UniquePtr(UniquePtr<S, D>&& other) noexcept
        : ptr_(std::forward<T*>(dynamic_cast<T*>(other.Get())),
               std::forward<Deleter>(*dynamic_cast<Deleter*>(&other.GetDeleter()))) {
        other.Clear();
    }

    template <typename S>
    UniquePtr(UniquePtr<S>&& other) noexcept
        : ptr_(std::forward<T*>(dynamic_cast<T*>(other.Get())),
               std::forward<Deleter>(other.GetDeleter())) {
        other.Clear();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s with Templates

    template <typename S, typename D>
    UniquePtr& operator=(UniquePtr<S, D>&& other) noexcept {
        if (dynamic_cast<T*>(other.Get()) == ptr_.GetFirst()) {
            return *this;
        }
        Destructor();
        ptr_ = CompressedPair<T*, Deleter>(
            std::forward<T*>(dynamic_cast<T*>(other.Get())),
            std::forward<Deleter>(*dynamic_cast<Deleter*>(&other.GetDeleter())));
        other.Clear();
        return *this;
    }

    template <typename S>
    UniquePtr& operator=(UniquePtr<S>&& other) noexcept {
        if (dynamic_cast<T*>(other.Get()) == ptr_.GetFirst()) {
            return *this;
        }
        Destructor();
        ptr_ = CompressedPair<T*, Deleter>(std::forward<T*>(dynamic_cast<T*>(other.Get())),
                                           std::forward<Deleter>(other.GetDeleter()));
        other.Clear();
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    void Destructor() {
        if (Get() != nullptr) {
            GetDeleter()(Get());
        }
        Clear();
    }

    ~UniquePtr() {
        Destructor();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    T* Release() {
        auto ptr = Get();
        Clear();
        return ptr;
    }

    void Reset(T* ptr = nullptr) {
        if (ptr == ptr_.GetFirst()) {
            return;
        }
        auto temp = ptr_.GetFirst();
        ptr_ = CompressedPair<T*, Deleter>(ptr, Deleter());
        delete temp;
    }

    void Swap(UniquePtr& other) {
        std::swap(ptr_.GetFirst(), other.ptr_.GetFirst());
        std::swap(ptr_.GetSecond(), other.ptr_.GetSecond());
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const {
        return ptr_.GetFirst();
    }
    Deleter& GetDeleter() {
        return ptr_.GetSecond();
    }

    const Deleter& GetDeleter() const {
        return ptr_.GetSecond();
    }

    Deleter& GetCD() {
        return ptr_;
    }

    const Deleter& GetCD() const {
        return ptr_;
    }

    explicit operator bool() const {
        return ptr_.GetFirst() != nullptr;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Single-object dereference operators

    auto operator*() const {
        return *Get();
    }

    T* operator->() {
        return Get();
    }

    const T* operator->() const {
        return Get();
    }
};

// Specialization for arrays
template <typename T, typename Deleter>
class UniquePtr<T[], Deleter> {
private:
    CompressedPair<T*, Deleter> ptr_;

    void Clear() {
        ptr_.GetFirst() = nullptr;
        ptr_.GetSecond() = Deleter();
    }

public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    explicit UniquePtr(T* ptr = nullptr) : ptr_(ptr, Deleter()) {
    }

    UniquePtr(T* ptr, const Deleter& deleter) : ptr_(ptr, deleter) {
    }

    UniquePtr(T* ptr, Deleter&& deleter) : ptr_(ptr, std::forward<Deleter>(deleter)) {
    }

    UniquePtr(UniquePtr&& other) noexcept
        : ptr_(std::forward<CompressedPair<T*, Deleter>>(other.ptr_)) {
        other.Clear();
    }

    UniquePtr(const UniquePtr& other) = delete;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    UniquePtr& operator=(UniquePtr&& other) noexcept {
        if (other.ptr_.GetFirst() == ptr_.GetFirst()) {
            return *this;
        }
        Destructor();
        ptr_ = std::forward<CompressedPair<T*, Deleter>>(other.ptr_);
        other.Clear();
        return *this;
    }
    UniquePtr& operator=(std::nullptr_t) {
        Destructor();
        return *this;
    }

    UniquePtr& operator=(const UniquePtr& other) = delete;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    void Destructor() {
        if (Get() != nullptr) {
            GetDeleter()(Get());
        }
        Clear();
    }

    ~UniquePtr() {
        Destructor();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    T* Release() {
        auto ptr = Get();
        Clear();
        return ptr;
    }

    void Reset(T* ptr = nullptr) {
        if (ptr == ptr_.GetFirst()) {
            return;
        }
        auto temp = ptr_.GetFirst();
        ptr_ = CompressedPair<T*, Deleter>(ptr, Deleter());
        delete[] temp;
    }

    void Swap(UniquePtr& other) {
        std::swap(ptr_.GetFirst(), other.ptr_.GetFirst());
        std::swap(ptr_.GetSecond(), other.ptr_.GetSecond());
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const {
        return ptr_.GetFirst();
    }

    const Deleter& GetDeleter() const {
        return ptr_.GetSecond();
    }

    Deleter& GetDeleter() {
        return ptr_.GetSecond();
    }

    explicit operator bool() const {
        return ptr_.GetFirst() != nullptr;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Single-object dereference operators

    auto operator*() const {
        return *Get();
    }

    T& operator[](size_t i) {
        return Get()[i];
    }

    const T& operator[](size_t i) const {
        return Get()[i];
    }

    T* operator->() {
        return Get();
    }

    const T* operator->() const {
        return Get();
    }
};
