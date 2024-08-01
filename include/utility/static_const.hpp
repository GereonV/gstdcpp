#ifndef GSTD_UTILITY_STATIC_CONST_HPP
#define GSTD_UTILITY_STATIC_CONST_HPP

// Until C++23, `operator()` & `operator[]` weren't allowed to be `static`.
// Use `GSTD_STATIC` where `static` should be, but previously couldn't.
// Similarly, `GSTD_CONST` should be used, where a `const` would've been previously.


#if __cplusplus >= 202101 && (__clang_major__ >= 16 || __GNUC__ >= 13)
#define GSTD_STATIC static
#define GSTD_CONST
#else
#define GSTD_STATIC
#define GSTD_CONST const
#endif

#endif
