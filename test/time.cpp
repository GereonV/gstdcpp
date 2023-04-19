#include <chrono>
#include <iostream>
#include <thread>
#include <time.hpp>

static void debug_timer() noexcept {
	gstd::debug_timer timer{"Timer Name"};
	std::this_thread::sleep_for(std::chrono::seconds{1});
	std::cout << "Debug Timer:\n";
}

static void debug_timer2() noexcept {
	gstd::debug_timer timer;
	std::this_thread::sleep_for(std::chrono::seconds{1});
	std::cout << "Debug Timer (unnamed):\n";
}

int main() {
	std::jthread t1{debug_timer};
	std::this_thread::sleep_for(std::chrono::milliseconds{100});
	std::jthread t2{debug_timer2};
}
