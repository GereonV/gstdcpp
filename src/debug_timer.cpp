#include "time.hpp"
#include <iostream>

gstd::debug_timer::debug_timer(char const * name) noexcept
	: _name{name},
	  _start_time{std::chrono::high_resolution_clock::now()} {}

gstd::debug_timer::~debug_timer() {
	auto end_time = std::chrono::high_resolution_clock::now();
	auto elapsed = end_time - _start_time;
	std::cout << "\27[2K\r"; // clear entire line and return
	if(_name)
		std::cout << _name << ": ";
	std::cout << elapsed << std::endl;
}
