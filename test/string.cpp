#include <iostream>
#include <vector>
#include <string.hpp>

struct custom_allocator {
	std::vector<int> allocations;

	constexpr auto allocate(size_t c) noexcept {
		allocations.emplace_back(static_cast<int>(c));
		return gstd::string::allocator::allocate(c);
	}

	constexpr auto reallocate(char * op, size_t oc, size_t nc) noexcept {
		allocations.emplace_back(-static_cast<int>(oc));
		allocations.emplace_back(static_cast<int>(nc));
		return gstd::string::allocator::reallocate(op, oc, nc);
	}

	constexpr void deallocate(char * p, size_t c) noexcept {
		allocations.emplace_back(-static_cast<int>(c));
		gstd::string::allocator::deallocate(p, c);
	}
};

consteval void constructors() noexcept {
	using gstd::local_string;
	using lstr_t = local_string<256>;
	using sstr_t = local_string<1>;
	{
		auto test = [](auto const & s) {
			return !s.size() && !*s.begin();
		};
		constexpr lstr_t lstr;
		constexpr sstr_t sstr;
		static_assert(test(lstr));
		static_assert(test(lstr_t{lstr}));
		static_assert(test(lstr_t{sstr}));
		static_assert(test(lstr_t{std::move(lstr_t{})}));
		static_assert(test(sstr));
		static_assert(test(sstr_t{sstr}));
		static_assert(test(sstr_t{lstr}));
		static_assert(test(sstr_t{std::move(sstr_t{})}));
	}
	{
		auto test = [](auto const & s) {
			if(s.size() != 10)
				return false;
			for(auto c : s)
				if(c != 'A')
					return false;
			return s[10] == 0;
		};
		static_assert(test(lstr_t{10, 'A'}));
		static_assert(test(lstr_t{sstr_t{10, 'A'}}));
		static_assert(test(lstr_t{std::move(lstr_t{10, 'A'})}));
		static_assert(test(lstr_t{"AAAAAAAAAA", 10}));
		static_assert(test(lstr_t{"AAAAAAAAAA"}));
		static_assert(test(sstr_t{10, 'A'}));
		static_assert(test(sstr_t{lstr_t{10, 'A'}}));
		static_assert(test(sstr_t{std::move(sstr_t{10, 'A'})}));
		static_assert(test(sstr_t{"AAAAAAAAAA", 10}));
		static_assert(test(sstr_t{"AAAAAAAAAA"}));
		constexpr local_string s{"Test"};
	}
}

int main() {
	constructors();
}
