#ifndef GSTD_ALLOCATION_BASE_HPP
#define GSTD_ALLOCATION_BASE_HPP

#include <concepts>
#include <type_traits>

namespace gstd::allocation {
    using size_t = decltype(sizeof(nullptr));

    template<typename T>
    struct creation_result {
        T * ptr;
        size_t size; // in bytes

        [[nodiscard]] constexpr operator bool() const noexcept { return ptr; }
    };

    using allocation_result = creation_result<void>;
    inline constexpr allocation_result no_allocation{nullptr, 0};

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
    concept ownership_aware_allocator = allocator<Alloc> && requires(Alloc const a) {
        { a.owns(allocation_result{nullptr, 0}) } -> std::same_as<bool>;
        requires noexcept(a.owns(allocation_result{nullptr, 0}));
    };

    template<typename Alloc>
    concept stateless_allocator = std::is_default_constructible_v<Alloc> && allocator<Alloc const>
                                  && (!reallocating_allocator<Alloc> || reallocating_allocator<Alloc const>)
                                  && (!ownership_aware_allocator<Alloc> || ownership_aware_allocator<Alloc const>);
}

#endif // GSTD_ALLOCATION_HPP
