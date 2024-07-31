#ifndef GSTD_META_TYPE_SEQUENCE_HPP
#define GSTD_META_TYPE_SEQUENCE_HPP

#include <type_traits>
#include "meta/predicate.hpp"
#include "utility/static_false.hpp"

namespace gstd::meta::type_sequence {
    using size_t = decltype(sizeof(nullptr));

    template<typename... Ts>
    struct type_sequence;
    using empty = type_sequence<>;

    namespace _impl {
        template<typename... Ts>
        struct not_type_sequences {
            static_assert(GSTD_STATIC_FALSE(Ts...), "every template argument must be a type_sequence");
        };

        template<size_t Idx, typename Head, typename... Tail>
        struct _get : _get<Idx - 1, Tail...> {};

        template<typename Head, typename... Tail>
        struct _get<0, Head, Tail...> {
            using type = Head;
        };

        // wrapper to provide a nicer error
        template<size_t Idx, typename... Ts>
        struct get {
            static_assert(Idx < sizeof...(Ts), "index out of bounds");
            using type = _get<Idx, Ts...>::type;
        };

        template<typename Seq, typename... Seqs>
        struct concat : not_type_sequences<Seq, Seqs...> {};

        template<typename... Ts>
        struct concat<type_sequence<Ts...>> {
            using type = type_sequence<Ts...>;
        };

        template<typename... Ts, typename... Us, typename... Tail>
        struct concat<type_sequence<Ts...>, type_sequence<Us...>, Tail...>
            : concat<type_sequence<Ts..., Us...>, Tail...> {};

        template<template<typename> typename Pred, typename... Ts>
        struct filter : concat<std::conditional_t<predicate::invoke<Pred, Ts>, type_sequence<Ts>, empty>...> {};

        template<typename... Ts>
        struct reversed {
            static_assert(sizeof...(Ts) == 0);
            using type = empty;
        };

        template<typename Head, typename... Tail>
        struct reversed<Head, Tail...> : concat<typename reversed<Tail...>::type, type_sequence<Head>> {};

        template<typename T>
        struct same_predicate {
            template<typename U>
            using type = std::is_same<T, U>;
        };

        template<typename... Ts>
        struct contains_predicate {
            template<typename U>
            using type = std::bool_constant<type_sequence<Ts...>::template contains<U>>;
        };

        template<typename Seq, typename... Seqs>
        struct intersection : not_type_sequences<Seq, Seqs...> {};

        template<typename... Ts>
        struct intersection<type_sequence<Ts...>> {
            using type = type_sequence<Ts...>;
        };

        template<typename... Ts, typename... Us, typename... Tail>
        struct intersection<type_sequence<Ts...>, type_sequence<Us...>, Tail...>
            : intersection<typename filter<contains_predicate<Us...>::template type, Ts...>::type, Tail...> {};

        template<typename Seq1, typename Seq2>
        struct difference : not_type_sequences<Seq1, Seq2> {};

        template<typename... Ts, typename... Us>
        struct difference<type_sequence<Ts...>, type_sequence<Us...>>
            : filter<predicate::negated<contains_predicate<Us...>::template type>::template type, Ts...> {};
    }

    template<typename... Seqs>
    using concat = _impl::concat<Seqs...>::type;

    template<typename... Seqs>
    using intersection = _impl::intersection<Seqs...>::type;

    template<typename Seq1, typename Seq2>
    using difference = _impl::difference<Seq1, Seq2>::type;

    template<typename Seq1, typename Seq2>
    using symmetric_difference = concat<difference<Seq1, Seq2>, difference<Seq2, Seq1>>;

    template<typename... Ts>
    struct type_sequence {
        static constexpr size_t size = sizeof...(Ts);
        static constexpr bool empty  = !size;

        template<size_t Idx>
        using get = _impl::get<Idx, Ts...>::type;

        template<typename... Us>
        using append = concat<type_sequence, type_sequence<Us...>>;

        template<template<typename> typename Pred>
        using filter = _impl::filter<Pred, Ts...>::type;

        template<template<typename> typename Pred>
        static constexpr bool any = !filter<Pred>::empty;

        template<template<typename> typename Pred>
        static constexpr bool all = filter<Pred>::size == size;

        using reversed = _impl::reversed<Ts...>::type;

        template<typename T>
        static constexpr bool contains = any<_impl::same_predicate<T>::template type>;

        template<typename... Us>
        using remove = difference<type_sequence, type_sequence<Us...>>;
    };
}

#endif
