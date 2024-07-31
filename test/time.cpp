#include <chrono>
#include <iostream>
#include <thread>
#include <time.hpp>

using gstd::time::debug_timer;

static void test_debug_timer() noexcept
{
    debug_timer timer{"Timer Name"};
    std::this_thread::sleep_for(std::chrono::seconds{1});
    std::cout << "Test debug_timer (1s):\n";
}

static void test_debug_timer_unnamed() noexcept
{
    debug_timer timer;
    std::this_thread::sleep_for(std::chrono::seconds{2});
    std::cout << "Test debug_timer unnamed (2s):\n";
}

int main()
{
    std::jthread t1{test_debug_timer};
    std::jthread t2{test_debug_timer_unnamed};
}
