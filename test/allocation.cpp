#include <algorithm>
#include <allocation.hpp>
#include <cassert>
#include <iostream>

using namespace gstd::allocation;

static_assert((
  []
  {
      auto res           = create<int>(c_allocator);
      auto res2          = create<int>(c_allocator, 42);
      auto [ptr, size]   = res;
      auto [ptr2, size2] = res2;
      assert(size == sizeof(int));
      assert(size2 == sizeof(int));
      assert(*ptr == 0);
      assert(*ptr2 == 42);
      destroy(c_allocator, res);
      destroy(c_allocator, res2);
  }(),
  true
));

struct S {
    S()    = default;
    S(S &) = delete;

    S(int const &, int &&) noexcept {}

    constexpr allocation_result allocate(size_t) const noexcept { return no_allocation; }

    constexpr void deallocate(allocation_result) const noexcept {}

    constexpr bool owns(allocation_result) const noexcept { return false; }
};

struct S2 : c_allocator_type {
    explicit S2() = default;

    S2(int, int &, int &&) noexcept {}
};

void test_fallback_and_arena() noexcept
{
    int i;
    fallback_allocator<S, S2> alloc; // = {} doesn't compile (explicit)
    fallback_allocator<S, S2> alloc2{
      std::piecewise_construct, std::forward_as_tuple(1, 2), std::forward_as_tuple(1, i, 3)
    };
    destroy(alloc, create<int>(alloc2, 17));

    arena_allocator<S2> alloc3{arena_size{4 * sizeof(int)}}; // = {arena_size{1}} doesn't compile (explicit)
    auto a1 = create<int>(alloc3, 17);
    auto a2 = create<int>(alloc3, 17);
    arena_allocator<S2> alloc4{std::move(alloc3)}; // {alloc3} doesn't compile (no copy)
    auto a3 = create<int>(alloc4, 17);
    auto a4 = create<int>(alloc4, 17);
    auto a5 = create<int>(alloc4, 17);
    assert(a1 && a2 && a3 && a4 && !a5); // just assume malloc(16) returns less than 20 bytes
    destroy(alloc4, a1);
    destroy(alloc4, a2);
    destroy(alloc4, a3);
    destroy(alloc4, a4);
}

int main()
{
    auto alloc = c_allocator;
    auto al    = alloc.allocate(1);
    std::cout << "Allocated 1 byte, got " << al.size << " @ " << al.ptr << '\n';
    auto re = alloc.reallocate(al, 4100);
    std::cout << "Reallocated 4100 bytes, got " << re.size << " @ " << re.ptr << '\n';
    alloc.deallocate(re);
    std::cout << "Deallocated\n";
    test_fallback_and_arena();
}
