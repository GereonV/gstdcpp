#include <iostream>
#include <allocation.hpp>

static gstd::default_allocator alloc;

consteval void constexpr_alloc() noexcept {
	auto result = alloc.allocate<char>(1);
	*result.ptr = 0; // memory must be initialized in constexpr context
	result.ptr = alloc.reallocate(result.ptr, 1, 4100);
	alloc.deallocate(result.ptr); // doesn't compile without this
}

int main() {
	constexpr_alloc();
	auto [ptr, count] = alloc.allocate<char>(1);
	std::cout << "Allocated 1 byte, got " << count;
	ptr = alloc.reallocate(ptr, 1, 4100);
	std::cout << "\nReallocated 4100 bytes\n";
	alloc.deallocate(ptr);
	std::cout << "Deallocated\n";
}
