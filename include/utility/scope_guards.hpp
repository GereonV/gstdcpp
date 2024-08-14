#ifndef GSTD_UTILITY_SCOPE_GUARDS_HPP
#define GSTD_UTILITY_SCOPE_GUARDS_HPP

// inspired by https://youtu.be/WjTrfoiB0MQ and std::experimental

#include <exception>
#include <utility/static_false.hpp>

#define GSTD_UTILITY_CONCAT_IMPL(a, b) a##b
#define GSTD_UTILITY_CONCAT(a, b)      GSTD_UTILITY_CONCAT_IMPL(a, b)

// macros don't work when used multiple times per line :(
#define GSTD_SCOPE_GUARD_IMPL(I)                                                               \
    [[maybe_unused]] auto const GSTD_UTILITY_CONCAT(_gstd_scope_guard, __LINE__)               \
      = ::gstd::utility::scope_guards::_impl::guard_marker<::gstd::utility::scope_guards::I>{} \
    += ::gstd::utility::scope_guards::_impl::usage_guard                                       \
    {}
#define GSTD_SCOPE_EXIT    GSTD_SCOPE_GUARD_IMPL(exit)
#define GSTD_SCOPE_SUCCESS GSTD_SCOPE_GUARD_IMPL(success)
#define GSTD_SCOPE_FAILURE GSTD_SCOPE_GUARD_IMPL(failure)

namespace gstd::utility::scope_guards {
    inline constexpr int exit    = 0;
    inline constexpr int success = 1;
    inline constexpr int failure = 2;

    template<typename F, int I>
    requires requires(F f) { f(); }
    class scope_guard;

    namespace _impl {
        struct usage_guard {
            template<typename F>
            constexpr F && operator+=(F && f) const noexcept
            {
                return static_cast<F &&>(f);
            }
        };

        template<int I>
        struct guard_marker {
            static_assert(0 <= I && I <= 2);

            template<typename T = void>
            usage_guard operator+=(usage_guard) const noexcept
            {
                static_assert(GSTD_STATIC_FALSE(T), "must supply callable (eg. GSTD_SCOPE_EXIT += [] { free(ptr); })");
            }

            template<typename F>
            constexpr scope_guard<F, I> operator+=(F && f) const noexcept
            {
                return scope_guard<F, I>{static_cast<F &&>(f)};
            }
        };

        template<int I>
        class scope_guard_base {
            static_assert(I == success || I == failure);
          public:
            scope_guard_base() noexcept : _ex_count{std::uncaught_exceptions()} {}

            [[nodiscard]] bool should_execute() const noexcept
            {
                return (I == success) == (_ex_count == std::uncaught_exceptions());
            }
          private:
            int _ex_count;
        };

        template<>
        class scope_guard_base<exit> {
          public:
            [[nodiscard]] constexpr bool should_execute() const noexcept { return true; }
        };
    }

    template<typename F, int I>
    requires requires(F f) { f(); }
    class [[nodiscard]] scope_guard : _impl::scope_guard_base<I> {
      public:
        explicit constexpr scope_guard(F && f) : _f(static_cast<F &&>(f)) {}

        constexpr ~scope_guard() noexcept(I != success || noexcept(_f()))
        {
            if(this->should_execute())
                _f();
        }

        scope_guard(scope_guard const &)    = delete;
        void operator=(scope_guard const &) = delete;
      private:
        [[no_unique_address]] F _f;
    };
}

#endif
