#pragma once

#include <exception>

class BadWeakPtr : public std::exception {};

class ControlBlockBase {
public:
    virtual void IncreaseStrongCounter() {
    }

    virtual void DecreaseStrongCounter() {
    }

    virtual void IncreaseWeakCounter() {
    }

    virtual void DecreaseWeakCounter() {
    }

    virtual void Delete() {
    }

    virtual ~ControlBlockBase() {
    }

    virtual void Release() {
    }

    virtual void ReleaseWeak() {
    }

    virtual size_t UseCount() {
        return 0;
    }

    virtual size_t UseStrongCount() {
        return 0;
    }
};

template <typename T>
class SharedPtr;

template <typename T>
class WeakPtr;
