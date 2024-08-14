#ifndef GSTD_ALLOCATION_ARENA_ALLOCATOR_HPP
#define GSTD_ALLOCATION_ARENA_ALLOCATOR_HPP

#include <type_traits>
#include <utility>
#include "allocation/base.hpp"
#include "allocation/buffer_allocator.hpp"

namespace gstd::allocation {
    struct arena_size {
        explicit constexpr arena_size(size_t size) : value{size} {}

        [[nodiscard]] constexpr operator size_t() const noexcept { return value; }

        size_t value;
    };

    namespace _impl {
        template<allocator Alloc>
        class arena_allocator_base {
          public:
            template<typename... Args>
            explicit(sizeof...(Args) == 0) constexpr arena_allocator_base(
              arena_size size,
              Args &&... args
            ) noexcept(std::is_nothrow_constructible_v<Alloc, Args...>)
                : _alloc(static_cast<Args &&>(args)...), _allocation{_alloc.allocate(size)}
            {}

            constexpr arena_allocator_base(arena_allocator_base && other
            ) noexcept(std::is_nothrow_constructible_v<Alloc, Alloc>)
                : _alloc(std::move(other._alloc)), _allocation{std::exchange(other._allocation, no_allocation)}
            {}

            constexpr arena_allocator_base & operator=(arena_allocator_base && rhs
            ) noexcept(std::is_nothrow_constructible_v<Alloc, Alloc>)
            {
                clear();
                _alloc      = std::move(rhs._alloc);
                _allocation = std::exchange(rhs._allocation, no_allocation);
                return *this;
            }

            constexpr ~arena_allocator_base() { clear(); }

            arena_allocator_base(arena_allocator_base const &) = delete;
            void operator=(arena_allocator_base const &)       = delete;
          protected:
            constexpr allocation_result get_allocation() const noexcept { return _allocation; }
          private:
            constexpr void clear() noexcept
            {
                if(_allocation)
                    _alloc.deallocate(std::exchange(_allocation, no_allocation));
            }

            [[no_unique_address]] Alloc _alloc;
            allocation_result _allocation;
        };
    }

    template<allocator Alloc>
    struct arena_allocator : _impl::arena_allocator_base<Alloc>,
                             _impl::buffer_allocator_base<arena_allocator<Alloc>> {
        friend _impl::buffer_allocator_base<arena_allocator<Alloc>>;
        using _impl::arena_allocator_base<Alloc>::arena_allocator_base;
    };
}

#endif
