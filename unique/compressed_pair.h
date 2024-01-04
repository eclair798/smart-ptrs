#pragma once

#include <type_traits>
#include <algorithm>

template <class T, size_t I, bool = std::is_empty_v<T> && !std::is_final_v<T>>
class CompressedPairElement {
public:
    CompressedPairElement() = default;

    CompressedPairElement(const T& value) {
        if constexpr (std::is_copy_constructible_v<T>) {
            value_ = T(value);
        }
    }

    CompressedPairElement(T& value) {
        if constexpr (std::is_copy_constructible_v<T>) {
            value_ = T(value);
        } else if constexpr (std::is_move_constructible_v<T>) {
            value_ = std::forward<T>(value);
        }
    }

    CompressedPairElement(T&& value) : value_(std::forward<T>(value)) {
    }

    T& Get() {
        return value_;
    }
    const T& GetConst() const {
        return value_;
    }

private:
    T value_;
};

template <class T, size_t I>
class CompressedPairElement<T, I, true> : T {
public:
    CompressedPairElement() = default;

    CompressedPairElement(const T&) {
    }
    CompressedPairElement(T&&) {
    }

    T& Get() {
        return *this;
    }

    const T& GetConst() const {
        return *this;
    }
};

template <typename F, typename S>
class CompressedPair : CompressedPairElement<F, 0>, CompressedPairElement<S, 1> {
public:
    CompressedPair() : CompressedPairElement<F, 0>(), CompressedPairElement<S, 1>() {
    }

    CompressedPair(const F& first, const S& second)
        : CompressedPairElement < F,
    0, std::is_empty_v<F> && !std::is_final_v < F >> (first), CompressedPairElement<S, 1>(second) {
    }

    CompressedPair(CompressedPair&& pair)
        : CompressedPairElement<F, 0>(std::forward<F>(pair.GetFirst())),
          CompressedPairElement<S, 1>(std::forward<S>(pair.GetSecond())) {
    }

    CompressedPair(F&& first, S&& second)
        : CompressedPairElement<F, 0>(std::forward<F>(first)),
          CompressedPairElement<S, 1>(std::forward<S>(second)) {
    }

    CompressedPair(const F& first, S&& second)
        : CompressedPairElement<F, 0>(first), CompressedPairElement<S, 1>(std::forward<S>(second)) {
    }

    CompressedPair(F&& first, const S& second)
        : CompressedPairElement<F, 0>(std::forward<F>(first)), CompressedPairElement<S, 1>(second) {
    }

    void operator=(CompressedPair&& pair) {
        GetFirst() = std::forward<F>(pair.GetFirst());
        GetSecond() = std::forward<S>(pair.GetSecond());
    }

    void operator=(const CompressedPair& pair) {
        GetFirst() = pair.GetFirst();
        GetSecond() = pair.GetSecond();
    }

    F& GetFirst() {
        return CompressedPairElement<F, 0>::Get();
    }

    const F& GetFirst() const {
        return CompressedPairElement<F, 0>::GetConst();
    }

    S& GetSecond() {
        return CompressedPairElement<S, 1>::Get();
    }

    const S& GetSecond() const {
        return CompressedPairElement<S, 1>::GetConst();
    }

    //    void Clear() {
    //        CompressedPairElement<S, 1>::Get()
    //    }
};