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
        {}

        template<typename T = Primary, typename U = Fallback>
        constexpr fallback_allocator(
          T && primary,
          U && fallback
        ) noexcept(std::is_nothrow_constructible_v<Primary, T> && std::is_nothrow_constructible_v<Fallback, U>)
            : _primary(static_cast<T &&>(primary)), _fallback(static_cast<U &&>(fallback))
        {}

        // eg. usable with std::forward_as_tuple
        template<typename PrimaryArgsTup, typename FallbackArgsTup>
        constexpr fallback_allocator(std::piecewise_construct_t, PrimaryArgsTup && p, FallbackArgsTup && f) noexcept(
          // all big STLs voluntarily provide correct `noexcept`-specification
          noexcept(std::make_from_tuple<Primary>(static_cast<PrimaryArgsTup &&>(p))) && //
          noexcept(std::make_from_tuple<Fallback>(static_cast<FallbackArgsTup &&>(f)))
        )
            : _primary{std::make_from_tuple<Primary>(static_cast<PrimaryArgsTup &&>(p))}
            , _fallback{std::make_from_tuple<Fallback>(static_cast<FallbackArgsTup &&>(f))}
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
