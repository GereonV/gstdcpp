#include <iostream>
#include <allocation.hpp>

int main() {
	auto [ptr, count] = gstd::do_allocation(1);
	std::cout << "Allocated 1 byte, got " << count;
	ptr = gstd::do_reallocation(ptr, 4100);
	std::cout << "\nReallocated 4100 bytes\n";
	gstd::do_deallocation(ptr);
	std::cout << "Deallocated\n";
}
