#pragma once

#include <type_traits>

namespace dr
{

struct TaskRef
{
    constexpr TaskRef() = default;

    /// Creates a task reference from a function object
    template <typename Src>
    constexpr TaskRef(Src* const src)
    {
        static_assert(std::is_invocable_r_v<void, Src>);

        if (src != nullptr)
        {
            if constexpr (std::is_const_v<Src>)
                ptr_ = const_cast<void*>(static_cast<void const*>(src));
            else
                ptr_ = src;

            invoke_ = [](void* ptr) { (*static_cast<Src*>(ptr))(); };
        }
    }

    /// Returns an opaque pointer to the referenced function object
    constexpr void* get() const { return ptr_; }

    /// Invokes the referenced task
    constexpr void operator()() const { invoke_(ptr_); }

    /// Returns true if the instance refers to a valid memory address
    constexpr bool is_valid() const { return invoke_ != nullptr; }
    constexpr explicit operator bool() const { return is_valid(); }

  private:
    void* ptr_{};
    void (*invoke_)(void*){};
};

} // namespace dr