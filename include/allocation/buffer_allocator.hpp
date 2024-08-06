#ifndef GSTD_ALLOCATION_BUFFER_ALLOCATOR_HPP
#define GSTD_ALLOCATION_BUFFER_ALLOCATOR_HPP

#include <bit>
#include <utility>
#include "allocation/base.hpp"

namespace gstd::allocation::_impl {
    template<typename Self>
    class buffer_allocator_base {
      public:
        constexpr buffer_allocator_base() noexcept : _top{_allocation().ptr} {}

        [[nodiscard]] constexpr allocation_result allocate(size_t size) noexcept
        {
            auto allocation = _allocation();
            if(!allocation)
                return no_allocation;
            auto remaining = (static_cast<char *>(allocation.ptr) + allocation.size) - static_cast<char *>(_top);
            if(size > static_cast<size_t>(remaining))
                return no_allocation;
            return {std::exchange(_top, static_cast<char *>(_top) + size), size};
        }

        // true deallocation is done at destruction
        constexpr void deallocate(allocation_result allocation) noexcept
        {
            // possibly free up space at the end
            if(static_cast<char *>(_top) == static_cast<char *>(allocation.ptr) + allocation.size)
                _top = allocation.ptr;
        }

        // this is technically not guaranteed to work... :(
        // could use implementation-defined total pointer ordering but that's not guaranteed to do the right thing either...
        // TODO maybe this should be preprocessor-guarded somehow?
        [[nodiscard]] constexpr bool owns(allocation_result allocation) const noexcept
        {
            auto [begin_ptr, size] = _allocation();
            auto begin             = std::bit_cast<std::uintptr_t>(begin_ptr);
            auto end               = std::bit_cast<std::uintptr_t>(static_cast<char *>(begin_ptr) + size);
            auto ptr               = std::bit_cast<std::uintptr_t>(allocation.ptr);
            // works because `allocation` has to be `.allocate()`-ed by some allocator and not `deallocate()`-ed yet ie. valid
            return begin <= ptr && ptr < end;
        }
      private:
        constexpr allocation_result _allocation() noexcept { return static_cast<Self *>(this)->get_allocation(); }

        void * _top;
    };
}

#endif
