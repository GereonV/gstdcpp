#include <array>
#include <iterator>
#include <ranges.hpp>

using namespace gstd;

static constinit int const array[]{1, 2, 3, 4, 5};

static_assert(ranges::begin(array) == array);
static_assert(ranges::end(array) == array + sizeof array / sizeof *array);
static_assert(ranges::rbegin(array) == std::reverse_iterator{array + sizeof array / sizeof *array});
static_assert(ranges::rend(array) == std::reverse_iterator{array});
static_assert(ranges::size(array) == sizeof array / sizeof *array);

static constinit std::array array2{1, 2, 3, 4, 5};
static_assert(ranges::begin(array2) == array2.data());
static_assert(ranges::end(array2) == array2.data() + array2.size());
static_assert(ranges::rbegin(array2) == array2.rbegin());
static_assert(ranges::rend(array2) == array2.rend());
static_assert(ranges::size(array2) == array2.size());

namespace test {
    static constinit char test[]        = "test";
    inline constexpr char const * ctest = "ctest";
    inline constexpr char * sent        = test + sizeof test;
    inline constexpr char const * csent = ctest + sizeof "ctest";
    inline constexpr size_t size_       = 42;
    inline constexpr size_t csize       = 17;

    template<typename T>
    struct rev {
        T value;

        constexpr operator T() const noexcept { return value; }
    };

    template<typename T>
    rev(T) -> rev<T>;

    template<typename T>
    struct normal_members_only {
        constexpr auto begin() noexcept { return test; }

        constexpr auto end() noexcept { return sent; }
    };

    template<typename T>
    struct members_only {
        constexpr auto begin() /* */ noexcept { return test; }

        constexpr auto begin() const noexcept { return ctest; }

        constexpr auto end() /* */ noexcept { return sent; }

        constexpr auto end() const noexcept { return csent; }

        constexpr auto rbegin() /* */ noexcept { return rev{sent}; }

        constexpr auto rbegin() const noexcept { return rev{csent}; }

        constexpr auto rend() /* */ noexcept { return rev{test}; }

        constexpr auto rend() const noexcept { return rev{ctest}; }

        constexpr auto size() /* */ noexcept { return test::size_; }

        constexpr auto size() const noexcept { return csize; }
    };

    template<typename T>
    struct minimal_adl_only {};

    template<typename T>
    constexpr auto begin(minimal_adl_only<T> &) noexcept
    {
        return test;
    }

    template<typename T>
    constexpr auto end(minimal_adl_only<T> &) noexcept
    {
        return sent;
    }

    template<typename T>
    struct adl_only {};

    template<typename T>
    constexpr auto begin(adl_only<T> /* */ &) noexcept
    {
        return test;
    }

    template<typename T>
    constexpr auto begin(adl_only<T> const &) noexcept
    {
        return ctest;
    }

    template<typename T>
    constexpr auto end(adl_only<T> /* */ &) noexcept
    {
        return sent;
    }

    template<typename T>
    constexpr auto end(adl_only<T> const &) noexcept
    {
        return csent;
    }

    template<typename T>
    constexpr auto rbegin(adl_only<T> &) noexcept
    {
        return nullptr;
    }

    template<typename T>
    constexpr auto rend(adl_only<T> &) noexcept
    {
        return nullptr;
    }

    template<typename T>
    constexpr auto size(adl_only<T> &) noexcept
    {
        return 36;
    }
}

static constinit test::normal_members_only<int> nmo;
static_assert(ranges::begin(nmo) == test::test);
static_assert(ranges::end(nmo) == test::sent);
static_assert(ranges::rbegin(nmo) == std::reverse_iterator{test::sent});
static_assert(ranges::rend(nmo) == std::reverse_iterator{test::test});
static_assert(ranges::size(nmo) == test::sent - test::test);
// ranges::begin(static_cast<test::normal_members_only<int> const>(nmo)) doesn't compile
// ranges::end(static_cast<test::normal_members_only<int> const>(nmo)) doesn't comile
// ranges::rbegin(static_cast<test::normal_members_only<int> const>(nmo)) doesn't compile
// ranges::rend(static_cast<test::normal_members_only<int> const>(nmo)) doesn't comile
// ranges::size(static_cast<test::normal_members_only<int> const>(nmo)) doesn't comile

static constinit test::members_only<int> mo;
static_assert(ranges::begin(mo) == test::test);
static_assert(ranges::end(mo) == test::sent);
static_assert(ranges::rbegin(mo) == test::rev{test::sent});
static_assert(ranges::rend(mo) == test::rev{test::test});
static_assert(ranges::size(mo) == test::size_);
static_assert(ranges::begin(static_cast<test::members_only<int> const &>(mo)) == test::ctest);
static_assert(ranges::end(static_cast<test::members_only<int> const &>(mo)) == test::csent);
static_assert(ranges::rbegin(static_cast<test::members_only<int> const &>(mo)) == test::rev{test::csent});
static_assert(ranges::rend(static_cast<test::members_only<int> const &>(mo)) == test::rev{test::ctest});
static_assert(ranges::size(static_cast<test::members_only<int> const &>(mo)) == test::csize);

static constinit test::minimal_adl_only<int> mao;
static_assert(ranges::begin(mao) == test::test);
static_assert(ranges::end(mao) == test::sent);
static_assert(ranges::rbegin(mao) == std::reverse_iterator{test::sent});
static_assert(ranges::rend(mao) == std::reverse_iterator{test::test});
static_assert(ranges::size(mao) == test::sent - test::test);
// ranges::begin(static_cast<test::minimal_adl_only<int> const &>(mao)) doesn't compile
// ranges::end(static_cast<test::minimal_adl_only<int> const &>(mao)) doesn't compile
// ranges::rbegin(static_cast<test::minimal_adl_only<int> const &>(mao)) doesn't compile
// ranges::rend(static_cast<test::minimal_adl_only<int> const &>(mao)) doesn't compile
// ranges::size(static_cast<test::minimal_adl_only<int> const &>(mao)) doesn't compile

static constinit test::adl_only<int> ao;
static_assert(ranges::begin(ao) == test::test);
static_assert(ranges::end(ao) == test::sent);
static_assert(ranges::rbegin(ao) == nullptr);
static_assert(ranges::rend(ao) == nullptr);
static_assert(ranges::size(ao) == 36);
static_assert(ranges::begin(static_cast<test::adl_only<int> const &>(ao)) == test::ctest);
static_assert(ranges::end(static_cast<test::adl_only<int> const &>(ao)) == test::csent);
static_assert(ranges::rbegin(static_cast<test::adl_only<int> const &>(ao)) == std::reverse_iterator{test::csent});
static_assert(ranges::rend(static_cast<test::adl_only<int> const &>(ao)) == std::reverse_iterator{test::ctest});
static_assert(ranges::size(static_cast<test::adl_only<int> const &>(ao)) == test::csent - test::ctest);
