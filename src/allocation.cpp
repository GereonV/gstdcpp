#include "allocation.hpp"
#include <cstdlib>

#include <malloc.h>
#define ALLOCATED_SIZE(ptr) malloc_usable_size(ptr)
// _msize on Windows
// malloc_size on MacOS

gstd::allocation_result<void> gstd::do_allocation(gstd::size_t size) noexcept {
	auto ptr = std::malloc(size);
	return {ptr, ALLOCATED_SIZE(ptr)};
}

void * gstd::do_reallocation(void * ptr, gstd::size_t new_size) noexcept {
	return std::realloc(ptr, new_size);
}

void gstd::do_deallocation(void * ptr) noexcept {
	std::free(ptr);
}
