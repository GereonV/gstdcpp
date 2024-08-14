#include <cassert>
#include <iostream>
#include <utility/scope_guards.hpp>
#include <utility>

void test()
{
    auto fail1       = [] noexcept { assert(false); };
    auto const fail2 = fail1;
    auto fail3       = +fail1;
    GSTD_SCOPE_EXIT += [] { std::cout << "guard 1\n"; };
    int x{42};
    std::cout << "x=" << x << "\n";
    auto f = [&x, y = x] { std::cout << "guard 2: initial x=" << y << " - current x=" << x << '\n'; };
    GSTD_SCOPE_EXIT += std::move(f);
    x = 17;
    std::cout << "x=" << x << "\n";
    GSTD_SCOPE_FAILURE += fail1;
    GSTD_SCOPE_SUCCESS += [] { std::cout << "guard 3\n"; };
    try {
        GSTD_SCOPE_SUCCESS += fail2;
        GSTD_SCOPE_EXIT += [] { std::cout << "nested guard 1\n"; };
        GSTD_SCOPE_FAILURE += [] { std::cout << "nested guard 2\n"; };
        throw static_cast<char const *>("example");
        GSTD_SCOPE_EXIT += fail1;
        GSTD_SCOPE_SUCCESS += fail2;
        GSTD_SCOPE_FAILURE += fail3;
    } catch(char const * const & ex) {
        auto f = [&] noexcept { std::cout << "try guard 1: " << ex << "\n"; };
        GSTD_SCOPE_SUCCESS += std::move(f);
        GSTD_SCOPE_EXIT += [] { std::cout << "try guard 2\n"; };
        GSTD_SCOPE_FAILURE += fail1;
    }
    GSTD_SCOPE_SUCCESS += [=]
    {
        GSTD_SCOPE_EXIT += [] { std::cout << "lambda guard 1\n"; };
        GSTD_SCOPE_SUCCESS += [] { std::cout << "lambda guard 2\n"; };
        GSTD_SCOPE_FAILURE += fail1;
        try {
            GSTD_SCOPE_FAILURE += []
            {
                assert(std::uncaught_exceptions() == 1);
                try {
                    GSTD_SCOPE_FAILURE += [] { assert(std::uncaught_exceptions() == 2); };
                    throw -2;
                } catch(...) {}
            };
            throw -1;
        } catch(...) {}
    };
}

void test_termination()
{
    try {
        GSTD_SCOPE_SUCCESS += [] { assert(false); };
        throw 42;
    } catch(...) {}
    try {
        GSTD_SCOPE_FAILURE += [] { throw "this should be the termination-cause"; };
        throw 42;
    } catch(...) {}
}

int main()
{
    /* expected result:
     * x=42
     * x=17
     * nested guard 2
     * nested guard 1
     * try guard 2
     * try guard 1: example
     * lambda guard 2
     * lambda guard 1
     * guard 3
     * guard 2: initial x=42 - current x=17
     * guard 1
     */
    test();
    // GSTD_SCOPE_EXIT; doesn't compile
    test_termination();
}
