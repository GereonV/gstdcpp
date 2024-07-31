#include <algorithm>
#include <allocation.hpp>
#include <cassert>
#include <iostream>

using namespace gstd::allocation;

static_assert((
  []
  {
      auto alloc         = c_allocator;
      auto res           = create<int>(alloc);
      auto res2          = create<int>(alloc, 42);
      auto [ptr, size]   = res;
      auto [ptr2, size2] = res2;
      assert(size == sizeof(int));
      assert(size2 == sizeof(int));
      assert(*ptr == 0);
      assert(*ptr2 == 42);
      destroy(alloc, res);
      destroy(alloc, res2);
  }(),
  true
));

int main()
{
    auto alloc = c_allocator;
    auto al    = alloc.allocate(1);
    std::cout << "Allocated 1 byte, got " << al.size << " @ " << al.ptr << '\n';
    auto re = alloc.reallocate(al, 4100);
    std::cout << "Reallocated 4100 bytes, got " << re.size << " @ " << re.ptr << '\n';
    alloc.deallocate(re);
    std::cout << "Deallocated\n";
}
