#include <concepts>
#include <meta/type_sequence.hpp>
#include <type_traits>

using namespace gstd::meta::type_sequence;

using seq  = type_sequence<void, char, short, int, long long>;
using seq2 = type_sequence<void, void *, void const *>;

static_assert(sizeof(short) == 2 && sizeof(long long) == 8, "used below as assumptions for testing");

template<typename T>
struct Predicate : std::bool_constant<sizeof(T) == 2 || sizeof(T) == 8> {};

template<>
struct Predicate<void> {};

template<typename T, typename U>
struct Comparator : std::bool_constant<sizeof(T) < sizeof(U)> {};

template<typename T>
struct Projection : std::type_identity<T *> {};

static_assert(empty::size == 0 && empty::empty);
static_assert(seq::size == 5 && !seq::empty);
static_assert(seq2::size == 3 && !seq::empty);
static_assert(std::same_as<seq::reversed, type_sequence<long long, int, short, char, void>>);
static_assert(std::same_as<seq::reversed::reversed, seq>);
static_assert(std::same_as<seq2::get<0>, void>);
static_assert(std::same_as<seq2::get<1>, void *>);
static_assert(std::same_as<seq2::get<2>, void const *>);
// seq2::get<3> doesn't compile
static_assert(std::same_as<
              concat<empty, seq, empty, empty, seq2, empty>,
              type_sequence<void, char, short, int, long long, void, void *, void const *>>);
static_assert(std::same_as<
              seq::append<void, void *, void const *>,
              type_sequence<void, char, short, int, long long, void, void *, void const *>>);
static_assert(std::same_as<intersection<seq, seq2::append<int>>, type_sequence<void, int>>);
static_assert(std::same_as<seq::filter<Predicate>, type_sequence<short, long long>>);
static_assert(seq::filter<Predicate>::all<Predicate>);
static_assert(!seq::filter<Predicate>::append<char>::all<Predicate>);
static_assert(!type_sequence<>::any<Predicate>);
static_assert(!type_sequence<int>::any<Predicate>);
static_assert(type_sequence<short>::any<Predicate>);
static_assert(type_sequence<short, int>::any<Predicate>);
static_assert(seq::contains<short> && !seq::contains<void *>);
static_assert(std::same_as<seq::remove<void, long long>, type_sequence<char, short, int>>);
static_assert(std::same_as<
              symmetric_difference<seq, seq2::append<int>>,
              type_sequence<char, short, long long, void *, void const *>>);
static_assert(concat<seq, seq2>::nth_index<void> == 0);
static_assert(concat<seq, seq2>::nth_index<void, 2> == seq::size);
// concat<seq, seq2>::nth_index<void, 3> doesn't compile
// seq::nth_index<int, 0> doesn't compile
static_assert(std::same_as<
              type_sequence<void, char, void, short, void, int, void, int>::unique,
              type_sequence<void, char, short, int>>);
static_assert(std::same_as<empty::sorted<Comparator>, empty>);
static_assert(std::same_as<type_sequence<void *, char>::sorted<Comparator>, type_sequence<char, void *>>);
static_assert(std::same_as<seq::remove<void>::sorted<Comparator>, seq::remove<void>>);
// seq::sorted<Comparator> doesn't compile
static_assert(std::same_as<seq2::mapped<Projection>, type_sequence<void *, void **, void const **>>);

int main() {}
