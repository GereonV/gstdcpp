#ifndef GSTD_STATIC_FALSE_HPP
#define GSTD_STATIC_FALSE_HPP

// Until C++23, `static_assert(false)` is always ill-formed.
// Use `static_assert(GSTD_STATIC_FALSE(T))` with some template-dependant type `T` as a workaround!
// Further information: https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2022/p2593r0.html


#if __cplusplus >= 202101 && (__clang_major__ >= 17 || __GNUC__ >= 13)
#define GSTD_STATIC_FALSE(T, ...) false
#else
#define GSTD_STATIC_FALSE(T, ...) gstd::utility::static_false::always_false_v<T __VA_OPT__(, ) __VA_ARGS__>

namespace gstd::utility::static_false {
    // This mustn't be specialized!
    // However, it needs to be specializable for the semantics to work.
    template<typename...>
    struct always_false {
        static constexpr bool value = false;
    };

    template<typename... T>
    inline constexpr bool always_false_v = always_false<T...>::value;
}
#endif
#endif
