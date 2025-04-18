#ifndef UNIQUE_PTR_HPP
#define UNIQUE_PTR_HPP

#include <utility>

namespace ptrs::unique {

template <typename T>
struct default_delete {
    void operator()(T *ptr) const noexcept {
        delete ptr;
    }
};

template <typename T, typename Deleter = default_delete<T>>
class unique_ptr {
    T *ptr_;
    Deleter deleter_;

public:
    unique_ptr() noexcept : ptr_(nullptr), deleter_() {
    }

    unique_ptr(std::nullptr_t  // cppcheck-suppress[noExplicitConstructor]
    ) noexcept
        : ptr_(nullptr), deleter_() {
    }

    explicit unique_ptr(T *ptr) noexcept : ptr_(ptr), deleter_() {
    }

    unique_ptr(const unique_ptr &) = delete;
    unique_ptr &operator=(const unique_ptr &) = delete;

    unique_ptr(T *ptr, Deleter deleter) noexcept
        : ptr_(ptr), deleter_(std::move(deleter)) {
    }

    unique_ptr(unique_ptr &&other) noexcept
        : ptr_(std::exchange(other.ptr_, nullptr)),
          deleter_(std::move(other.deleter_)) {
    }

    unique_ptr &operator=(unique_ptr &&other) noexcept {
        if (&other != this) {
            reset();
            ptr_ = other.release();
            deleter_ = std::move(other.deleter_);
        }
        return *this;
    }

    ~unique_ptr() {
        reset();
    }

    void reset(T *ptr = nullptr) noexcept {
        if (ptr_ != ptr) {
            if (ptr_) {
                deleter_(ptr_);
            }
            ptr_ = ptr;
        }
    }

    T *release() noexcept {
        return std::exchange(ptr_, nullptr);
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

    void swap(unique_ptr &other) noexcept {
        std::swap(ptr_, other.ptr_);
        std::swap(deleter_, other.deleter_);
    }
};

template <typename T, typename Deleter>
void swap(
    unique_ptr<T, Deleter> &first_unique_ptr,
    unique_ptr<T, Deleter> &second_unique_ptr
) noexcept {
    first_unique_ptr.swap(second_unique_ptr);
}

template <typename T, typename Deleter>
bool operator==(
    const unique_ptr<T, Deleter> &first_unique_ptr,
    const unique_ptr<T, Deleter> &second_unique_ptr
) noexcept {
    return first_unique_ptr.get() == second_unique_ptr.get();
}

template <typename T, typename Deleter>
bool operator!=(
    const unique_ptr<T, Deleter> &first_unique_ptr,
    const unique_ptr<T, Deleter> &second_unique_ptr
) noexcept {
    return !(first_unique_ptr == second_unique_ptr);
}

}  // namespace ptrs::unique

#endif  // UNIQUE_PTR_HPP