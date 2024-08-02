#ifndef GSTD_META_TYPE_SEQUENCE_BASE_HPP
#define GSTD_META_TYPE_SEQUENCE_BASE_HPP

#include <type_traits>

namespace gstd::meta::type_sequence {
    template<typename... Ts>
    struct type_sequence;
    using empty  = type_sequence<>;
    using size_t = decltype(sizeof(nullptr));

    namespace _impl {
        template<typename T>
        struct is_type_sequence : std::false_type {};

        template<typename... Ts>
        struct is_type_sequence<type_sequence<Ts...>> : std::true_type {};

        template<typename T>
        inline constexpr bool is_type_sequence_v = is_type_sequence<T>::value;
    }

    template<typename T>
    concept sequence_of_types = _impl::is_type_sequence_v<T>;
}

#endif
