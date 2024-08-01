#ifndef GSTD_META_PREDICATE_HPP
#define GSTD_META_PREDICATE_HPP

#include <concepts>
#include <type_traits>

// A meta-programming (unary) predicate `Pred` is a class template that assigns a `Pred<T>::value` to types `T`.
// Special consideration is required if `Pred<T>::value` doesn't exist, often it is interpreted as `false`.

namespace gstd::meta::predicate {
    namespace _impl {
        template<typename T>
        concept has_bool_value = requires {
            { T::value } -> std::same_as<bool const &>;
        };

        template<typename T>
        concept has_no_value = !requires { T::value; };

        template<typename T, template<typename> typename Pred>
        concept predicated_by = has_bool_value<Pred<T>> || has_no_value<Pred<T>>;
    }

    // prevents SFINAE: `strict<Pred>::type<T>` cannot be instantiated unless `Pred<T>::value` exists
    template<template<typename> typename Pred>
    struct strict {
        template<_impl::predicated_by<Pred> T>
        struct type : Pred<T> {
            static_assert(
              _impl::has_bool_value<Pred<T>>,
              "predicate must have a value for all types it's instantiated with"
            );
        };
    };

    // `completed<Pred>::type<T>::value == Pred<T>::value`
    // SFINAE: if `Pred<T>::value` doesn't exist, `Default` is used
    template<template<typename> typename Pred, bool Default = false>
    struct completed {
        template<_impl::predicated_by<Pred> T>
        struct type : std::bool_constant<Default> {};

        template<typename T>
        requires _impl::has_bool_value<Pred<T>>
        struct type<T> : Pred<T> {};
    };

    // `negated<Pred>::type<T>::value == !Pred<T>::value`
    // SFINAE: if `Pred<T>::value` doesn't exist, `negated<Pred><T>::value` doesn't either
    template<template<typename> typename Pred>
    struct negated {
        template<_impl::predicated_by<Pred> T>
        struct type {};

        template<typename T>
        requires _impl::has_bool_value<Pred<T>>
        struct type<T> : std::bool_constant<!Pred<T>::value> {};
    };

    // `inverted<Pred>::type<T>::value == negated<completed<Pred>::type>::type<T>::value`
    template<template<typename> typename Pred>
    struct inverted : negated<completed<Pred>::template type> {};

    // uses return-type of templated `operator()`
    template<auto Lambda>
    struct from_lambda {
        template<typename T>
        struct type {};

        template<typename T>
        requires requires { Lambda.template operator()<T>(); }
        struct type<T> : decltype(Lambda.template operator()<T>()) {
            static_assert(_impl::has_bool_value<type>, "invalid return type");
        };
    };

    template<template<typename> typename Pred, typename T, bool Default = false>
    inline constexpr bool invoke = completed<Pred, Default>::template type<T>::value;
}

#endif
