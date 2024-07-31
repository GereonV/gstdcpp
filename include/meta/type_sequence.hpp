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
        template<typename T>
        struct is_type_sequence {
            static constexpr bool value = false;
        };

        template<typename... Ts>
        struct is_type_sequence<type_sequence<Ts...>> {
            static constexpr bool value = true;
        };
    }

    template<typename T>
    concept sequence_of_types = _impl::is_type_sequence<T>::value;

    namespace _impl {
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

        template<sequence_of_types... Seqs>
        struct concat {
            static_assert(sizeof...(Seqs) == 0);
            using type = empty;
        };

        template<typename... Ts>
        struct concat<type_sequence<Ts...>> {
            using type = type_sequence<Ts...>;
        };

        template<typename... Ts, typename... Us, sequence_of_types... Tail>
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
            using type = std::bool_constant<(std::same_as<Ts, U> || ...)>;
        };

        template<sequence_of_types... Seqs>
        struct intersection {
            static_assert(sizeof...(Seqs) == 0);
            using type = empty;
        };

        template<typename... Ts>
        struct intersection<type_sequence<Ts...>> {
            using type = type_sequence<Ts...>;
        };

        template<typename... Ts, typename... Us, sequence_of_types... Tail>
        struct intersection<type_sequence<Ts...>, type_sequence<Us...>, Tail...>
            : intersection<typename filter<contains_predicate<Us...>::template type, Ts...>::type, Tail...> {};

        template<sequence_of_types Seq1, sequence_of_types Seq2>
        struct difference {
            static_assert(GSTD_STATIC_FALSE(Seq1, Seq2), "should always use partial specialization");
        };

        template<typename... Ts, typename... Us>
        struct difference<type_sequence<Ts...>, type_sequence<Us...>>
            : filter<predicate::negated<contains_predicate<Us...>::template type>::template type, Ts...> {};
    }

    template<sequence_of_types... Seqs>
    using concat = _impl::concat<Seqs...>::type;

    template<sequence_of_types... Seqs>
    using intersection = _impl::intersection<Seqs...>::type;

    template<sequence_of_types Seq1, sequence_of_types Seq2>
    using difference = _impl::difference<Seq1, Seq2>::type;

    template<sequence_of_types Seq1, sequence_of_types Seq2>
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
        static constexpr bool contains = !intersection<type_sequence, type_sequence<T>>::empty;

        template<typename... Us>
        using remove = difference<type_sequence, type_sequence<Us...>>;

        // TODO get (n-th) index of type
        // TODO unique
        // TODO sort
        // TODO zip
    };
}

#endif
