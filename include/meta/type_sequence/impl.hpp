#ifndef GSTD_META_TYPE_SEQUENCE_IMPL_HPP
#define GSTD_META_TYPE_SEQUENCE_IMPL_HPP

#include <concepts>
#include <type_traits>
#include "meta/predicate.hpp"
#include "meta/type_sequence/base.hpp"
#include "utility/static_false.hpp"

namespace gstd::meta::type_sequence::_impl {
    // inherit from this if you're declaring a base template that has exhaustive specializations
    template<typename... Ts>
    struct error_base {
        static_assert(GSTD_STATIC_FALSE(Ts...), "should always use partial specialization");
    };

    template<sequence_of_types Seq>
    struct size : error_base<size<Seq>> {};

    template<typename... Ts>
    struct size<type_sequence<Ts...>> : std::integral_constant<size_t, sizeof...(Ts)> {};

    template<sequence_of_types Seq>
    inline constexpr size_t size_v = size<Seq>::value;

    template<sequence_of_types Seq>
    struct cons {
        static_assert(size_v<Seq> == 0);
    };

    template<typename Head, typename... Tail>
    struct cons<type_sequence<Head, Tail...>> {
        using head = Head;
        using tail = type_sequence<Tail...>;
    };

    template<sequence_of_types Seq>
    using head_t = cons<Seq>::head;

    template<sequence_of_types Seq>
    using tail_t = cons<Seq>::tail;

    template<sequence_of_types Seq, size_t Idx>
    struct get : get<tail_t<Seq>, Idx - 1> {};

    template<size_t N>
    struct get<empty, N> {};

    template<sequence_of_types Seq>
    requires (size_v<Seq> > 0)
    struct get<Seq, 0> : std::type_identity<head_t<Seq>> {};

    template<sequence_of_types Seq, size_t N>
    using get_t = get<Seq, N>::type;

    template<sequence_of_types Seq, template<typename> typename Proj>
    struct mapped : error_base<mapped<Seq, Proj>> {};

    template<typename... Ts, template<typename> typename Proj>
    struct mapped<type_sequence<Ts...>, Proj> : std::type_identity<type_sequence<typename Proj<Ts>::type...>> {};

    template<sequence_of_types Seq, template<typename> typename Proj>
    using mapped_t = mapped<Seq, Proj>::type;

    template<sequence_of_types... Seqs>
    struct concat : std::type_identity<empty> {
        static_assert(sizeof...(Seqs) == 0);
    };

    template<sequence_of_types Seq>
    struct concat<Seq> : std::type_identity<Seq> {};

    template<typename... Ts, typename... Us, sequence_of_types... Tail>
    struct concat<type_sequence<Ts...>, type_sequence<Us...>, Tail...> : concat<type_sequence<Ts..., Us...>, Tail...> {
    };

    template<sequence_of_types... Seqs>
    using concat_t = concat<Seqs...>::type;

    template<size_t N>
    struct indices : concat<typename indices<N - 1>::type, type_sequence<std::integral_constant<size_t, N - 1>>> {};

    template<>
    struct indices<0> : std::type_identity<empty> {};

    template<size_t N>
    using indices_t = indices<N>::type;

    template<sequence_of_types Seq, template<typename> typename Pred>
    struct filter : error_base<filter<Seq, Pred>> {};

    template<typename... Ts, template<typename> typename Pred>
    struct filter<type_sequence<Ts...>, Pred>
        : concat<std::conditional_t<predicate::invoke<Pred, Ts>, type_sequence<Ts>, empty>...> {};

    template<sequence_of_types Seq, template<typename> typename Pred>
    using filter_t = filter<Seq, Pred>::type;

    template<sequence_of_types Seq>
    struct reversed : concat<typename reversed<tail_t<Seq>>::type, type_sequence<head_t<Seq>>> {};

    template<>
    struct reversed<empty> : std::type_identity<empty> {};

    template<sequence_of_types Seq>
    using reversed_t = reversed<Seq>::type;

    template<sequence_of_types Seq>
    struct contains_predicate : error_base<contains_predicate<Seq>> {};

    template<typename... Ts>
    struct contains_predicate<type_sequence<Ts...>> {
        template<typename T>
        struct type : std::bool_constant<(std::same_as<T, Ts> || ...)> {};
    };

    template<sequence_of_types... Seqs>
    struct intersection : std::type_identity<empty> {
        static_assert(sizeof...(Seqs) == 0);
    };

    template<sequence_of_types Seq>
    struct intersection<Seq> : std::type_identity<Seq> {};

    template<typename Seq, typename Head, sequence_of_types... Tail>
    struct intersection<Seq, Head, Tail...>
        : intersection<filter_t<Seq, contains_predicate<Head>::template type>, Tail...> {};

    template<sequence_of_types... Seqs>
    using intersection_t = intersection<Seqs...>::type;

    template<sequence_of_types Seq1, sequence_of_types Seq2>
    struct difference : filter<Seq1, predicate::negated<contains_predicate<Seq2>::template type>::template type> {};

    template<sequence_of_types Seq1, sequence_of_types Seq2>
    using difference_t = difference<Seq1, Seq2>::type;

    template<sequence_of_types Seq, typename T, size_t N = 1>
    requires (N > 0)
    struct nth_index
        : std::integral_constant<size_t, nth_index<tail_t<Seq>, T, N - std::same_as<head_t<Seq>, T>>::value + 1> {};

    template<sequence_of_types Seq, typename T>
    requires std::same_as<head_t<Seq>, T>
    struct nth_index<Seq, T, 1> : std::integral_constant<size_t, 0> {};

    template<typename T, size_t N>
    struct nth_index<empty, T, N> {};

    template<sequence_of_types Seq, typename T, size_t N = 1>
    inline constexpr size_t nth_index_v = nth_index<Seq, T, N>::value;

    template<sequence_of_types Seq>
    struct unique : concat<
                      type_sequence<head_t<Seq>>,
                      typename unique<difference_t<tail_t<Seq>, type_sequence<head_t<Seq>>>>::type> {};

    template<>
    struct unique<empty> : std::type_identity<empty> {};

    template<sequence_of_types Seq>
    using unique_t = unique<Seq>::type;

    template<template<typename, typename> typename Comp, typename T>
    struct compare_predicate {
        template<typename U>
        struct type : Comp<T, U> {};
    };

    template<template<typename, typename> typename Comp, typename T, sequence_of_types Seq>
    struct can_compare : error_base<can_compare<Comp, T, Seq>> {};

    template<template<typename, typename> typename Comp, typename T, typename... Ts>
      struct can_compare<Comp, T, type_sequence<Ts...>> : std::bool_constant <
        (requires {
        {
            Comp<T, Ts>::value
        } -> std::same_as<bool const &>; } && ...)
     > {};

    template<template<typename, typename> typename Comp, typename T, typename Seq>
    concept can_compare_v = can_compare<Comp, T, Seq>::value;

    template<sequence_of_types Seq, template<typename, typename> typename Comp, typename T>
    struct insert_sorted {};

    template<sequence_of_types Seq, template<typename, typename> typename Comp, typename T>
    requires can_compare_v<Comp, T, Seq>
    struct insert_sorted<Seq, Comp, T>
        : concat<
            filter_t<Seq, predicate::negated<compare_predicate<Comp, T>::template type>::template type>,
            type_sequence<T>,
            filter_t<Seq, compare_predicate<Comp, T>::template type>> {};

    template<sequence_of_types Seq, template<typename, typename> typename Comp, typename T>
    using insert_sorted_t = insert_sorted<Seq, Comp, T>::type;

    template<sequence_of_types Seq, template<typename, typename> typename Comp, sequence_of_types Seq2>
    struct insert_all_sorted {};

    template<sequence_of_types Seq, template<typename, typename> typename Comp, sequence_of_types Seq2>
    requires can_compare_v<Comp, head_t<Seq2>, Seq>
    struct insert_all_sorted<Seq, Comp, Seq2>
        : insert_all_sorted<insert_sorted_t<Seq, Comp, head_t<Seq2>>, Comp, tail_t<Seq2>> {};

    template<sequence_of_types Seq, template<typename, typename> typename Comp>
    struct insert_all_sorted<Seq, Comp, empty> : std::type_identity<Seq> {};

    template<sequence_of_types Seq, template<typename, typename> typename Comp, sequence_of_types Seq2>
    using insert_all_sorted_t = insert_all_sorted<Seq, Comp, Seq2>::type;

    template<sequence_of_types Seq, template<typename, typename> typename Comp>
    struct sort : insert_all_sorted<empty, Comp, Seq> {};

    template<sequence_of_types Seq, template<typename, typename> typename Comp>
    using sort_t = sort<Seq, Comp>::type;

    // this doesn't compile, even though it should afaik
    //     template<sequence_of_types Seq, sequence_of_types... Seqs>
    //     struct zip {};
    // doesn't matter anyway when using zip_t, so we settle for this:
    template<sequence_of_types Seq, typename... Seqs>
    struct zip {
        static_assert((sequence_of_types<Seqs> && ...));
    };

    template<sequence_of_types Seq, sequence_of_types... Seqs>
    requires ((size_v<Seq> == size_v<Seqs>) && ...)
    struct zip<Seq, Seqs...> : concat<
                                 type_sequence<type_sequence<head_t<Seq>, head_t<Seqs>...>>,
                                 typename zip<tail_t<Seq>, tail_t<Seqs>...>::type> {};

    template<std::same_as<empty>... Seqs>
    struct zip<empty, Seqs...> : std::type_identity<empty> {};

    template<sequence_of_types Seq, sequence_of_types... Seqs>
    using zip_t = zip<Seq, Seqs...>::type;

    // TODO reduce/accumulate/fold
}

#endif
