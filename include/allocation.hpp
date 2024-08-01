#ifndef GSTD_ALLOCATION_HPP
#define GSTD_ALLOCATION_HPP

#include <concepts>
#include <type_traits>

namespace gstd::allocation {
    using size_t = decltype(sizeof(nullptr));

    struct allocation_result {
        void * ptr;
        size_t size;
    };

    template<typename Alloc>
    concept allocator = requires(Alloc a) {
        { a.allocate(size_t{}) } -> std::same_as<allocation_result>;
        { a.deallocate(allocation_result{nullptr, 0}) } -> std::same_as<void>;
        requires noexcept(a.allocate(size_t{}));
        requires noexcept(a.deallocate(allocation_result{nullptr, 0}));
    };

    template<typename Alloc>
    concept reallocating_allocator = allocator<Alloc> && requires(Alloc a) {
        { a.reallocate(allocation_result{nullptr, 0}, size_t{}) } -> std::same_as<allocation_result>;
        requires noexcept(a.reallocate(allocation_result{nullptr, 0}, size_t{}));
    };

    template<typename Alloc>
    concept ownership_aware_allocator = allocator<Alloc> && requires(Alloc a) {
        { a.owns(allocation_result{nullptr, 0}) } -> std::same_as<bool>;
        requires noexcept(a.owns(allocation_result{nullptr, 0}));
    };

    // Use `stateless_allocator` for checking the properties of a type!
    // May be specialized so a type satisfies the `stateless_allocator` concept.
    // `is_stateless_allocator<T>::value` iff `T::is_stateless_allocator` exists and is truthy or if specialized
    template<typename T>
    struct is_stateless_allocator {
        static constexpr bool value = requires { requires T::is_stateless_allocator; };
    };

    template<typename Alloc>
    concept stateless_allocator
      = allocator<Alloc> && std::is_default_constructible_v<Alloc> && is_stateless_allocator<Alloc>::value;

    template<typename T>
    struct creation_result {
        T * ptr;
        size_t size; // still in bytes
    };

    template<typename T, typename... Args>
    requires (std::constructible_from<T, Args...>)
    constexpr creation_result<T>
    create(allocator auto & alloc, Args &&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>)
    {
        if consteval {
            return {new T(static_cast<Args &&>(args)...), sizeof(T)};
        } else {
            auto result = alloc.allocate(sizeof(T));
            if(!result.ptr)
                return {nullptr, 0};
            return {new (result.ptr) T(static_cast<Args &&>(args)...), result.size};
        }
    }

    template<typename T>
    constexpr void destroy(allocator auto & alloc, creation_result<T> creation) noexcept
    {
        if consteval {
            delete creation.ptr;
        } else {
            creation.ptr->~T();
            alloc.deallocate(allocation_result{creation.ptr, creation.size});
        }
    }

    // uses C allocation library (std::malloc, std::realloc, std::free)
    struct c_allocator_type {
        static constexpr bool is_stateless_allocator = true;
        [[nodiscard]] static allocation_result allocate(size_t size) noexcept;
        [[nodiscard]] static allocation_result reallocate(allocation_result allocation, size_t new_size) noexcept;
        void deallocate(allocation_result allocation) noexcept;
    };

    static_assert(reallocating_allocator<c_allocator_type>);
    static_assert(stateless_allocator<c_allocator_type>);

    inline constexpr c_allocator_type c_allocator;
}

#endif // GSTD_ALLOCATION_HPP
