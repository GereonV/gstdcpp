#ifndef GSTD_ALLOCATION_FALLBACK_ALLOCATOR_HPP
#define GSTD_ALLOCATION_FALLBACK_ALLOCATOR_HPP

#include <type_traits>
#include "allocation/base.hpp"
#include "utility/static_false.hpp"

namespace gstd::allocation {
    inline constexpr struct fallback_sep_t {
    } fallback_sep;

    template<ownership_aware_allocator Primary, allocator Fallback>
    class fallback_allocator {
        // using deducing this to avoid double implementations would be nice but could be misused via explicit template parameters :/
        static constexpr bool stateless = stateless_allocator<Primary> && stateless_allocator<Fallback>;
      public:
        fallback_allocator() = default;

        template<typename... PrimaryArgs, typename... FallbackArgs>
        explicit(sizeof...(PrimaryArgs) + sizeof...(FallbackArgs) == 0) constexpr fallback_allocator(
          PrimaryArgs &&... p,
          fallback_sep_t,
          FallbackArgs &&... f
        ) noexcept(std::is_nothrow_constructible_v<Primary, PrimaryArgs...> && std::is_nothrow_constructible_v<Fallback, FallbackArgs...>)
            : _primary{static_cast<PrimaryArgs &&>(p)...}, _fallback{static_cast<FallbackArgs &&>(f)...}
        {}

        constexpr allocation_result allocate(size_t size) noexcept
        requires (!stateless)
        {
            if(auto allocation = _primary.allocate(size))
                return allocation;
            return _fallback.allocate(size);
        }

        constexpr allocation_result allocate(size_t size) const noexcept
        requires stateless
        {
            if(auto allocation = _primary.allocate(size))
                return allocation;
            return _fallback.allocate(size);
        }

        constexpr void deallocate(allocation_result allocation) noexcept
        requires (!stateless)
        {
            if(_primary.owns(allocation))
                _primary.deallocate(allocation);
            _fallback.deallocate(allocation);
        }

        constexpr void deallocate(allocation_result allocation) const noexcept
        requires stateless
        {
            if(_primary.owns(allocation))
                _primary.deallocate(allocation);
            _fallback.deallocate(allocation);
        }

        constexpr allocation_result reallocate(allocation_result allocation, size_t size) noexcept
        requires (!stateless && reallocating_allocator<Primary> && reallocating_allocator<Fallback>)
        {
            if(_primary.owns(allocation))
                return _primary.reallocate(allocation, size);
            return _fallback.reallocate(allocation, size);
        }

        constexpr allocation_result reallocate(allocation_result allocation, size_t size) noexcept
        requires (stateless && reallocating_allocator<Primary> && reallocating_allocator<Fallback>)
        {
            if(_primary.owns(allocation))
                return _primary.reallocate(allocation, size);
            return _fallback.reallocate(allocation, size);
        }

        constexpr bool owns(allocation_result allocation) const noexcept
        requires (ownership_aware_allocator<Fallback>)
        {
            return _primary.owns(allocation) || _fallback.owns(allocation);
        }
      private:
        [[no_unique_address]] Primary _primary;
        [[no_unique_address]] Fallback _fallback;
    };
}

#endif
