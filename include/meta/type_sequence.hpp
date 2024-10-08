#ifndef GSTD_META_TYPE_SEQUENCE_HPP
#define GSTD_META_TYPE_SEQUENCE_HPP

#include <concepts>
#include "meta/type_sequence/base.hpp"
#include "meta/type_sequence/impl.hpp"
#include "meta/type_sequence/sort.hpp"

namespace gstd::meta::type_sequence {
    template<std::integral Int, Int... Ints>
    using integer_sequence = type_sequence<std::integral_constant<Int, Ints>...>;

    template<size_t... Ints>
    using index_sequence = integer_sequence<size_t, Ints...>;

    template<size_t N>
    using indices = _impl::indices_t<N>;

    template<sequence_of_types... Seqs>
    using concat = _impl::concat_t<Seqs...>;

    template<sequence_of_types... Seqs>
    using intersection = _impl::intersection_t<Seqs...>;

    template<sequence_of_types Seq1, sequence_of_types Seq2>
    using difference = _impl::difference_t<Seq1, Seq2>;

    template<sequence_of_types Seq1, sequence_of_types Seq2>
    using symmetric_difference = concat<difference<Seq1, Seq2>, difference<Seq2, Seq1>>;

    template<sequence_of_types Seq, sequence_of_types... Seqs>
    using zip = _impl::zip_t<Seq, Seqs...>;

    template<typename... Ts>
    struct type_sequence {
        static constexpr size_t size = _impl::size_v<type_sequence>;
        static constexpr bool empty  = !size;

        template<size_t Idx>
        using get = _impl::get_t<type_sequence, Idx>;

        using head = _impl::head_t<type_sequence>;
        using tail = _impl::tail_t<type_sequence>;

        template<typename... Us>
        using append = concat<type_sequence, type_sequence<Us...>>;

        template<template<typename> typename Pred>
        using filter = _impl::filter_t<type_sequence, Pred>;

        template<template<typename> typename Pred>
        static constexpr bool any = !filter<Pred>::empty;

        template<template<typename> typename Pred>
        static constexpr bool all = filter<Pred>::size == size;

        using reversed = _impl::reversed_t<type_sequence>;

        template<typename T>
        static constexpr bool contains = !intersection<type_sequence<T>, type_sequence>::empty;

        template<typename... Us>
        using remove = difference<type_sequence, type_sequence<Us...>>;

        template<typename T, size_t N = 1>
        static constexpr size_t nth_index = _impl::nth_index_v<type_sequence, T, N>;

        using unique = _impl::unique_t<type_sequence>;

        template<template<typename, typename> typename Comp>
        using sorted = _impl::sort_t<type_sequence, Comp>;

        template<template<typename> typename Proj>
        using mapped = _impl::mapped_t<type_sequence, Proj>;

        using enumerated = zip<indices<size>, type_sequence>;
    };

    // TODO insert at index
}

#endif
