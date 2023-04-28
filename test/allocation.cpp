#include <cassert>
#include <iostream>
#include <allocation.hpp>

consteval void comptime() {
	using gstd::string::allocator;
	auto [ptr, count] = allocator::allocate(10);
	auto p = ptr;
	assert(ptr && count == 10 && ptr[9] == 0);
	ptr[9] = 'A';
	allocator::reallocate(ptr, 10, 20);
	assert(p != ptr && ptr[9] == 'A');
	p = ptr;
	allocator::reallocate(ptr, 20, 5);
	assert(p != ptr);
	p = ptr;
	allocator::reallocate(ptr, 5, 10);
	assert(p != ptr && ptr[9] == 0);
	allocator::deallocate(ptr, 10);
}

int main() {
	auto [ptr, count] = gstd::do_allocation(1);
	std::cout << "Allocated 1 byte, got " << count;
	ptr = gstd::do_reallocation(ptr, 4100);
	std::cout << "\nReallocated 4100 bytes\n";
	gstd::do_deallocation(ptr);
	std::cout << "Deallocated\n";
	comptime();
}
