#include "allocation.hpp"

#include <cstdlib>

#ifdef _WIN32
#error `_msize` not supported yet
// _msize on Windows
#elif __linux__
#include <malloc.h>
#define ALLOCATED_SIZE(ptr) malloc_usable_size(ptr)
#elif __APPLE__
#include <malloc/malloc.h>
#define ALLOCATED_SIZE(ptr) malloc_size(ptr)
#else
#warning No known function to query size of allocation returned from `malloc`
#endif

namespace gstd::allocation {
    allocation_result c_allocator_type::allocate(size_t size) noexcept
    {
        auto ptr = std::malloc(size);
        return {ptr, ALLOCATED_SIZE(ptr)};
    }

    allocation_result c_allocator_type::reallocate(allocation_result allocation, size_t new_size) noexcept
    {
        auto ptr = std::realloc(allocation.ptr, new_size);
        return {ptr, ALLOCATED_SIZE(ptr)};
    }

    void c_allocator_type::deallocate(allocation_result allocation) noexcept { std::free(allocation.ptr); }
}
