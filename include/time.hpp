#ifndef GSTD_TIME_HPP
#define GSTD_TIME_HPP

#include <chrono>

namespace gstd {

	class debug_timer {
	public:
		debug_timer(char const * name = nullptr) noexcept;
		debug_timer(debug_timer const &) = delete;
		~debug_timer();
	private:
		char const * _name;
		std::chrono::time_point<std::chrono::high_resolution_clock> _start_time;
	};

}

#endif // GSTD_TIME_HPP
