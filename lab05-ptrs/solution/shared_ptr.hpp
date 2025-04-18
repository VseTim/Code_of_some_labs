#ifndef SHARED_PTR_HPP
#define SHARED_PTR_HPP

#include <cstddef>
#include <utility>

namespace ptrs::shared {
template <typename T>
class shared_ptr {
    T *ptr_;
    std::size_t *ref_count_;

public:
    shared_ptr() noexcept : ptr_(nullptr), ref_count_(nullptr) {
    }

    explicit shared_ptr(std::nullptr_t) noexcept
        : ptr_(nullptr), ref_count_(nullptr) {
    }

    explicit shared_ptr(T *ptr) : ptr_(ptr), ref_count_(new std::size_t(1)) {
    }

    shared_ptr(                  // cppcheck-suppress[noExplicitConstructor]
        const shared_ptr &other  // cppcheck-suppress[noExplicitConstructor]
    ) noexcept                   // cppcheck-suppress[noExplicitConstructor]
        : ptr_(other.ptr_), ref_count_(other.ref_count_) {
        if (ref_count_ != nullptr) {
            (*ref_count_)++;
        }
    }

    shared_ptr(shared_ptr &&other) noexcept
        : ptr_(std::exchange(other.ptr_, nullptr)),
          ref_count_(std::exchange(other.ref_count_, nullptr)) {
    }

    void destroy() {
        std::size_t *rc = ref_count_;
        T *p = ptr_;
        if ((rc != nullptr) && --(*rc) == 0) {
            delete p;
            delete rc;
        }
    }

    ~shared_ptr() {
        destroy();
    }

    void reset(T *ptr = nullptr) {
        if (ptr_ != ptr) {
            if ((ref_count_ != nullptr) && --(*ref_count_) == 0) {
                delete ptr_;
                delete ref_count_;
            }
            ptr_ = ptr;
            if (ptr_ != nullptr) {
                ref_count_ = new std::size_t(1);
            } else {
                ref_count_ = nullptr;
            }
        }
    }

    shared_ptr &operator=(const shared_ptr &other) noexcept {
        if (&other != this) {
            // reset();
            if ((ref_count_ != nullptr) && --(*ref_count_) == 0) {
                delete ptr_;
                delete ref_count_;
                // reset();
            }
            ptr_ = other.ptr_;
            ref_count_ = other.ref_count_;
            if (ref_count_ != nullptr) {
                (*ref_count_)++;
            }
        }
        return *this;
    }

    shared_ptr &operator=(shared_ptr &&other) noexcept {
        if (this != &other) {
            if ((ref_count_ != nullptr) && --(*ref_count_) == 0) {
                delete ptr_;
                delete ref_count_;
            }
            ptr_ = std::exchange(other.ptr_, nullptr);
            ref_count_ = std::exchange(other.ref_count_, nullptr);
        }
        return *this;
    }

    T *get() const noexcept {
        return ptr_;
    }

    T &operator*() const noexcept {
        return *ptr_;
    }

    T *operator->() const noexcept {
        return ptr_;
    }

    explicit operator bool() const noexcept {
        return ptr_ != nullptr;
    }

    void swap(shared_ptr &other) noexcept {
        std::swap(ptr_, other.ptr_);
        std::swap(ref_count_, other.ref_count_);
    }
};

template <typename T>
void swap(
    shared_ptr<T> &first_unique_ptr,
    shared_ptr<T> &second_unique_ptr
) noexcept {
    first_unique_ptr.swap(second_unique_ptr);
}

template <typename T>
bool operator==(
    const shared_ptr<T> &first_unique_ptr,
    const shared_ptr<T> &second_unique_ptr
) noexcept {
    return first_unique_ptr.get() == second_unique_ptr.get();
}

template <typename T>
bool operator!=(
    const shared_ptr<T> &first_unique_ptr,
    const shared_ptr<T> &second_unique_ptr
) noexcept {
    return !(first_unique_ptr == second_unique_ptr);
}
}  // namespace ptrs::shared
#endif  // SHARED_PTR_HPP