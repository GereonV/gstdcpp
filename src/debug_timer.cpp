#include "time.hpp"

#include <iostream>

namespace gstd::time {
    debug_timer::debug_timer(char const * name) noexcept
        : _name{name}, _start_time{std::chrono::high_resolution_clock::now()}
    {}

    debug_timer::~debug_timer()
    {
        display();
        std::cout << '\n';
    }

    void debug_timer::display() const
    {
        auto end_time = std::chrono::high_resolution_clock::now();
        auto elapsed  = end_time - _start_time;
        std::cout << "\27[2K\r"                                         // clear entire line and return
                  << (_name ? _name : "debug_timer") << ": " << elapsed // print name and elapsed time
                  << std::flush;                                        // flush stream so output is shown
    }
}
