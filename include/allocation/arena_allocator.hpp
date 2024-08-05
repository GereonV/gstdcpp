#ifndef GSTD_ALLOCATION_ARENA_ALLOCATOR_HPP
#define GSTD_ALLOCATION_ARENA_ALLOCATOR_HPP

#include <bit>
#include <utility>
#include "allocation/base.hpp"

namespace gstd::allocation {
    struct arena_size {
        explicit constexpr arena_size(size_t size) : value{size} {}

        [[nodiscard]] constexpr operator size_t() const noexcept { return value; }

        size_t value;
    };

    template<allocator Alloc>
    class arena_allocator {
      public:
        // TODO check whether this can be implicitly chosen (as casting for argument-passing for example)
        template<typename... Args>
        constexpr arena_allocator(arena_size size, Args &&... args) //
          noexcept(noexcept(Alloc{static_cast<Args &&>(args)...}))
            : /*        */ _alloc{static_cast<Args &&>(args)...}
            , _allocation{_alloc.allocate(size)}
            , _top{_allocation.ptr}
        {}

        // TODO does this compile if Alloc doesn't support move-construction?
        constexpr arena_allocator(arena_allocator && other) //
          noexcept(noexcept(Alloc{std::move(other._alloc)}))
            : /*        */ _alloc{std::move(other._alloc)}
            , _allocation{std::exchange(other._allocation, no_allocation)}
            , _top{std::exchange(other._top, nullptr)}
        {}

        // TODO does this compile if Alloc doesn't support move-assignment?
        constexpr arena_allocator & operator=(arena_allocator && rhs) noexcept
        {
            clear();
            _alloc      = std::move(rhs._alloc);
            _allocation = std::exchange(rhs._allocation, no_allocation);
            _top        = std::exchange(rhs._top, nullptr);
            return *this;
        }

        ~arena_allocator() { clear(); }

        arena_allocator(arena_allocator const &) = delete;
        void operator=(arena_allocator const &)  = delete;

        constexpr void clear() noexcept
        {
            if(_allocation)
                _alloc.deallocate(_allocation);
            _allocation = no_allocation;
            _top        = nullptr;
        }

        [[nodiscard]] constexpr allocation_result allocate(size_t size) noexcept
        {
            auto remaining = (static_cast<char *>(_allocation.ptr) + _allocation.size) - static_cast<char *>(_top);
            if(size > remaining)
                return no_allocation;
            return {std::exchange(_top, static_cast<char *>(_top) + size), size};
        }

        // true deallocation is done at destruction
        constexpr void deallocate(allocation_result allocation) noexcept
        {
            if(static_cast<char *>(_top) == static_cast<char *>(allocation.ptr) + allocation.size)
                _top = allocation.ptr;
        }

        // this is technically not guaranteed to work... :(
        // TODO maybe this should be preprocessor-guarded somehow?
        [[nodiscard]] constexpr bool owns(allocation_result allocation) const noexcept
        {
            auto base = std::bit_cast<std::uintptr_t>(_allocation.ptr);
            auto ptr  = std::bit_cast<std::uintptr_t>(allocation.ptr);
#ifdef NDEBUG
            // works because `allocation` has to be `.allocate()`-ed by some allocator and not `deallocate()`-ed yet
            return base <= ptr && ptr < base + _allocation.size;
#else
            return base <= ptr && ptr + allocation.size <= base + _allocation.size;
#endif
        }
      private:
        Alloc _alloc;
        allocation_result _allocation;
        void * _top;
    };
}

#endif
