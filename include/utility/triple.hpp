#ifndef GSTD_UTILITY_TRIPLE_HPP
#define GSTD_UTILITY_TRIPLE_HPP

#define GSTD_TRIPLE(expr) GSTD_R_TRIPLE(decltype(expr), expr)

#define GSTD_R_TRIPLE(ret, expr) \
    noexcept(noexcept(expr))->ret { return expr; }

#endif
