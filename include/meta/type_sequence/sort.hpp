#ifndef GSTD_META_TYPE_SEQUENCE_SORT_HPP
#define GSTD_META_TYPE_SEQUENCE_SORT_HPP

#include "meta/type_sequence/base.hpp"
#include "meta/type_sequence/impl.hpp"

namespace gstd::meta::type_sequence::_impl {
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
}

#endif
