#ifndef GSTD_ALLOCATION_FALLBACK_ALLOCATOR_HPP
#define GSTD_ALLOCATION_FALLBACK_ALLOCATOR_HPP

#include <tuple>
#include <type_traits>
#include <utility>
#include "allocation/base.hpp"
#include "meta/type_sequence.hpp"
#include "utility/explicit_check.hpp"

namespace gstd::allocation {
    template<ownership_aware_allocator Primary, allocator Fallback>
    class fallback_allocator {
        // using deducing this to avoid double implementations would be nice but could be misused via explicit template parameters :/
        static constexpr bool stateless = stateless_allocator<Primary> && stateless_allocator<Fallback>;
      public:
        explicit(!utility::explicit_check::implicity_constructible_from<Primary> || !utility::explicit_check::implicity_constructible_from<Fallback>)
          fallback_allocator()
          = default;

        template<typename T = Primary, typename U = Fallback>
        constexpr fallback_allocator(
          T && primary,
          U && fallback
        ) noexcept(std::is_nothrow_constructible_v<Primary, T> && std::is_nothrow_constructible_v<Fallback, U>)
            : _primary{static_cast<T &&>(primary)}, _fallback{static_cast<U &&>(fallback)}
        {}

        // use with std::forward_as_tuple
        template<typename... PrimaryArgs, typename... FallbackArgs>
        constexpr fallback_allocator(
          std::piecewise_construct_t,
          std::tuple<PrimaryArgs...> p,
          std::tuple<FallbackArgs...> f
        ) noexcept(std::is_nothrow_constructible_v<Primary, PrimaryArgs...> && std::is_nothrow_constructible_v<Fallback, FallbackArgs...>)
            : fallback_allocator{
                p, f, meta::type_sequence::indices<sizeof...(PrimaryArgs)>{},
                meta::type_sequence::indices<sizeof...(FallbackArgs)>{}
              }
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
        template<typename PTup, typename FTup, meta::type_sequence::size_t... PI, meta::type_sequence::size_t... FI>
        constexpr fallback_allocator(PTup & p, FTup & f, meta::type_sequence::index_sequence<PI...>, meta::type_sequence::index_sequence<FI...>) noexcept(
          noexcept(Primary{get<PI>(std::move(p))...}) && noexcept(Fallback{get<FI>(std::move(f))...})
        )
            : _primary{get<PI>(std::move(p))...}, _fallback{get<FI>(std::move(f))...}
        {}

        [[no_unique_address]] Primary _primary;
        [[no_unique_address]] Fallback _fallback;
    };
}

#endif
