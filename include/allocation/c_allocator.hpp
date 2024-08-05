#ifndef GSTD_ALLOCATION_C_ALLOCATOR_HPP
#define GSTD_ALLOCATION_C_ALLOCATOR_HPP

#include "allocation/base.hpp"

namespace gstd::allocation {
    // uses C allocation library (std::malloc, std::realloc, std::free)
    struct c_allocator_type {
        static constexpr bool is_stateless_allocator = true;
        [[nodiscard]] static allocation_result allocate(size_t size) noexcept;
        [[nodiscard]] static allocation_result reallocate(allocation_result allocation, size_t new_size) noexcept;
        static void deallocate(allocation_result allocation) noexcept;
    };

    static_assert(reallocating_allocator<c_allocator_type>);
    static_assert(stateless_allocator<c_allocator_type>);
    inline constexpr c_allocator_type c_allocator;
}

#endif
