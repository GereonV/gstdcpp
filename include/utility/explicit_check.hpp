#ifndef GSTD_UTILITY_EXPLICIT_CHECK_HPP
#define GSTD_UTILITY_EXPLICIT_CHECK_HPP

namespace gstd::utility::explicit_check {
    template<typename T, typename... Args>
    concept implicity_constructible_from
      = requires(void (&func)(T), Args... args) { func({static_cast<Args &&>(args)...}); };
}

#endif
