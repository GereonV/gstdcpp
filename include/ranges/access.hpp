#ifndef GSTD_RANGES_ACCESS_HPP
#define GSTD_RANGES_ACCESS_HPP

#include <iterator>
#include "ranges/base.hpp"
#include "utility/static_const.hpp"

namespace gstd::ranges {
    namespace _impl::access {
        // shadows everything except exactly-matching ADL-overloads
        void begin(auto &&) = delete;
        void end(auto &&)   = delete;

        // TODO refine return type to iterator
        template<typename T>
        concept member_begin = requires(T t) { static_cast<T &&>(t).begin(); };
        template<typename T>
        concept member_end = requires(T t) { static_cast<T &&>(t).end(); };
        template<typename T>
        concept adl_begin = requires(T t) { begin(static_cast<T &&>(t)); };
        template<typename T>
        concept adl_end = requires(T t) { end(static_cast<T &&>(t)); };
        template<typename T>
        concept member_type = !adl_begin<T> && !adl_end<T> && member_begin<T> && member_end<T>;
        template<typename T>
        concept adl_type = adl_begin<T> && adl_end<T>;

        struct begin_fn {
            template<typename T, size_t N>
            [[nodiscard]] GSTD_STATIC constexpr T * operator()(T (&array)[N]) GSTD_CONST noexcept
            {
                return array;
            }

            template<member_type Range>
            [[nodiscard]] GSTD_STATIC constexpr auto operator()(Range && rng) GSTD_CONST //
              noexcept(noexcept(static_cast<Range &&>(rng).begin()))
              /* */ -> decltype(static_cast<Range &&>(rng).begin())
            {
                /*    */ return static_cast<Range &&>(rng).begin();
            }

            template<adl_type Range>
            [[nodiscard]] GSTD_STATIC constexpr auto operator()(Range && rng) GSTD_CONST //
              noexcept(noexcept(begin(static_cast<Range &&>(rng))))
              /* */ -> decltype(begin(static_cast<Range &&>(rng)))
            {
                /*    */ return begin(static_cast<Range &&>(rng));
            }
        };

        struct end_fn {
            template<typename T, size_t N>
            [[nodiscard]] GSTD_STATIC constexpr T * operator()(T (&array)[N]) GSTD_CONST noexcept
            {
                return array + N;
            }

            template<member_type Range>
            [[nodiscard]] GSTD_STATIC constexpr auto operator()(Range && rng) GSTD_CONST //
              noexcept(noexcept(static_cast<Range &&>(rng).end()))
              /* */ -> decltype(static_cast<Range &&>(rng).end())
            {
                /*    */ return static_cast<Range &&>(rng).end();
            }

            template<adl_type Range>
            [[nodiscard]] GSTD_STATIC constexpr auto operator()(Range && rng) GSTD_CONST //
              noexcept(noexcept(end(static_cast<Range &&>(rng))))
              /* */ -> decltype(end(static_cast<Range &&>(rng)))
            {
                /*    */ return end(static_cast<Range &&>(rng));
            }
        };
    }

    namespace _impl::raccess {
        // shadows everything except exactly-matching ADL-overloads
        void rbegin(auto &&) = delete;
        void rend(auto &&)   = delete;

        // TODO refine return type to iterator
        template<typename T>
        concept member_rbegin = requires(T t) { static_cast<T &&>(t).rbegin(); };
        template<typename T>
        concept member_rend = requires(T t) { static_cast<T &&>(t).rend(); };
        template<typename T>
        concept adl_rbegin = requires(T t) { rbegin(static_cast<T &&>(t)); };
        template<typename T>
        concept adl_rend = requires(T t) { rend(static_cast<T &&>(t)); };
        template<typename T>
        concept member_type = !adl_rbegin<T> && !adl_rend<T> && member_rbegin<T> && member_rend<T>;
        template<typename T>
        concept adl_type = adl_rbegin<T> && adl_rend<T>;
        template<typename T>
        concept access_type = !adl_rbegin<T> && !adl_rend<T> && !member_rbegin<T> && !member_rend<T> && requires(T t) {
            std::reverse_iterator{access::begin_fn{}(static_cast<T &&>(t))};
            std::reverse_iterator{access::end_fn{}(static_cast<T &&>(t))};
        };

        struct rbegin_fn {
            template<member_type Range>
            [[nodiscard]] GSTD_STATIC constexpr auto operator()(Range && rng) GSTD_CONST //
              noexcept(noexcept(static_cast<Range &&>(rng).rbegin()))
              /* */ -> decltype(static_cast<Range &&>(rng).rbegin())
            {
                /*    */ return static_cast<Range &&>(rng).rbegin();
            }

            template<adl_type Range>
            [[nodiscard]] GSTD_STATIC constexpr auto operator()(Range && rng) GSTD_CONST //
              noexcept(noexcept(rbegin(static_cast<Range &&>(rng))))
              /* */ -> decltype(rbegin(static_cast<Range &&>(rng)))
            {
                /*    */ return rbegin(static_cast<Range &&>(rng));
            }

            template<access_type Range>
            [[nodiscard]] GSTD_STATIC constexpr auto operator()(Range && rng) GSTD_CONST //
              noexcept(noexcept(std::reverse_iterator{access::end_fn{}(static_cast<Range &&>(rng))}))
              /* */ -> decltype(std::reverse_iterator{access::end_fn{}(static_cast<Range &&>(rng))})
            {
                /*    */ return std::reverse_iterator{access::end_fn{}(static_cast<Range &&>(rng))};
            }
        };

        struct rend_fn {
            template<member_type Range>
            [[nodiscard]] GSTD_STATIC constexpr auto operator()(Range && rng) GSTD_CONST //
              noexcept(noexcept(static_cast<Range &&>(rng).rend()))
              /* */ -> decltype(static_cast<Range &&>(rng).rend())
            {
                /*    */ return static_cast<Range &&>(rng).rend();
            }

            template<adl_type Range>
            [[nodiscard]] GSTD_STATIC constexpr auto operator()(Range && rng) GSTD_CONST //
              noexcept(noexcept(rend(static_cast<Range &&>(rng))))
              /* */ -> decltype(rend(static_cast<Range &&>(rng)))
            {
                /*    */ return rend(static_cast<Range &&>(rng));
            }

            template<access_type Range>
            [[nodiscard]] GSTD_STATIC constexpr auto operator()(Range && rng) GSTD_CONST //
              noexcept(noexcept(std::reverse_iterator{access::begin_fn{}(static_cast<Range &&>(rng))}))
              /* */ -> decltype(std::reverse_iterator{access::begin_fn{}(static_cast<Range &&>(rng))})
            {
                /*    */ return std::reverse_iterator{access::begin_fn{}(static_cast<Range &&>(rng))};
            }
        };
    }

    namespace _impl::size {
        // shadows everything except exactly-matching ADL-overloads
        void size(auto &&)  = delete;
        void ssize(auto &&) = delete;

        // TODO refine return type to (unsigned) integral
        template<typename T>
        concept member_size = requires(T t) { static_cast<T &&>(t).size(); };
        template<typename T>
        concept adl_size = requires(T t) { size(static_cast<T &&>(t)); };
        template<typename T>
        concept member_size_type = !adl_size<T> && member_size<T>;
        template<typename T>
        concept adl_size_type = adl_size<T>;
        template<typename T>
        concept access_size_type = !adl_size<T> && !member_size<T> && requires(T t) {
            std::distance(access::begin_fn{}(static_cast<T &&>(t)), access::end_fn{}(static_cast<T &&>(t)));
        };

        struct size_fn {
            template<member_size_type Range>
            [[nodiscard]] GSTD_STATIC constexpr auto operator()(Range && rng) GSTD_CONST //
              noexcept(noexcept(static_cast<Range &&>(rng).size()))
              /* */ -> decltype(static_cast<Range &&>(rng).size())
            {
                return /*    */ static_cast<Range &&>(rng).size();
            }

            template<adl_size_type Range>
            [[nodiscard]] GSTD_STATIC constexpr auto operator()(Range && rng) GSTD_CONST //
              noexcept(noexcept(size(static_cast<Range &&>(rng))))
              /* */ -> decltype(size(static_cast<Range &&>(rng)))
            {
                return /*    */ size(static_cast<Range &&>(rng));
            }

            template<access_size_type Range>
            [[nodiscard]] GSTD_STATIC constexpr auto operator()(Range && rng) GSTD_CONST //
              noexcept(noexcept(std::distance(
                access::begin_fn{}(static_cast<Range &&>(rng)),
                access::end_fn{}(static_cast<Range &&>(rng))
              )))
              /* */ -> decltype(std::distance(
                      access::begin_fn{}(static_cast<Range &&>(rng)),
                      access::end_fn{}(static_cast<Range &&>(rng))
                    ))
            {
                return std::distance(
                  access::begin_fn{}(static_cast<Range &&>(rng)), access::end_fn{}(static_cast<Range &&>(rng))
                );
            }
        };
    }

    namespace _impl::empty {
        // shadows everything except exactly-matching ADL-overloads
        void empty(auto &&) = delete;

        template<typename T>
        concept member_empty = requires(T t) {
            { static_cast<T &&>(t).empty() } -> std::convertible_to<bool>;
        };
        template<typename T>
        concept adl_empty = requires(T t) {
            { empty(static_cast<T &&>(t)) } -> std::convertible_to<bool>;
        };
        template<typename T>
        concept member_type = !adl_empty<T> && member_empty<T>;
        template<typename T>
        concept adl_type = adl_empty<T>;
        template<typename T>
        concept size_type
          = !adl_empty<T> && !member_type<T> && requires(T t) { size::size_fn{}(static_cast<T &&>(t)) == 0; };

        struct empty_fn {
            template<member_type Range>
            [[nodiscard]] GSTD_STATIC constexpr bool operator()(Range && rng) GSTD_CONST
              noexcept(noexcept(static_cast<Range &&>(rng).empty()))
            {
                return static_cast<Range &&>(rng).empty();
            }

            template<adl_type Range>
            [[nodiscard]] GSTD_STATIC constexpr bool operator()(Range && rng) GSTD_CONST
              noexcept(noexcept(empty(static_cast<Range &&>(rng))))
            {
                return empty(static_cast<Range &&>(rng));
            }

            template<size_type Range>
            [[nodiscard]] GSTD_STATIC constexpr bool operator()(Range && rng) GSTD_CONST
              noexcept(noexcept(size::size_fn{}(static_cast<Range &&>(rng)) == 0))
            {
                return size::size_fn{}(static_cast<Range &&>(rng)) == 0;
            }
        };
    }

    inline constexpr _impl::access::begin_fn begin;
    inline constexpr _impl::access::end_fn end;
    inline constexpr _impl::raccess::rbegin_fn rbegin;
    inline constexpr _impl::raccess::rend_fn rend;
    inline constexpr _impl::size::size_fn size;
    inline constexpr _impl::empty::empty_fn empty;
    // TODO ssize, data
}

#endif
