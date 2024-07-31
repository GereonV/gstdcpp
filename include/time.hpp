#ifndef GSTD_TIME_HPP
#define GSTD_TIME_HPP

#include <chrono>

namespace gstd::time {
    class debug_timer {
      public:
        debug_timer(char const * name = nullptr) noexcept;
        debug_timer(debug_timer const &)             = delete;
        debug_timer & operator=(debug_timer const &) = delete;
        // calls `display()` and inserts a newline
        ~debug_timer();
        // clears line and prints time since construction (no trailing newline)
        void display() const;
      private:
        char const * _name;
        std::chrono::time_point<std::chrono::high_resolution_clock> _start_time;
    };

    // TODO std::cout << debug_timer{} support
    // TODO logging timer (no clearing, support generic output stream)
}

#endif // GSTD_TIME_HPP
