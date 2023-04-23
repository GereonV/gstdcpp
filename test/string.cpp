#include <cassert>
#include <vector>
#include <string.hpp>

struct custom_allocator {
	std::string_view init{"Default"};
	std::vector<int> allocations;

	custom_allocator() = default;
	constexpr custom_allocator(std::string_view init) noexcept : init{init} {}

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
			return !s.size() && !*s.data();
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
		constexpr local_string<256, custom_allocator> cstr;
		constexpr local_string<256, custom_allocator> cstr2{gstd::allocator_arguments, "Custom"};
		constexpr local_string cstr3{cstr2};
		constexpr local_string cstr4{cstr2, gstd::allocator_arguments, cstr2.allocator()};
		constexpr local_string cstr5{cstr2, gstd::allocator_arguments, "Custom 2"};
		static_assert(test(cstr));
		static_assert(test(cstr2));
		static_assert(test(cstr3));
		static_assert(test(cstr4));
		static_assert(test(cstr5));
		static_assert(cstr.allocator().init == "Default");
		static_assert(cstr2.allocator().init == "Custom");
		static_assert(cstr3.allocator().init == "Default");
		static_assert(cstr4.allocator().init == "Custom");
		static_assert(cstr5.allocator().init == "Custom 2");
		static_assert(test(local_string{std::move(local_string{cstr})}));
		static_assert(local_string{std::move(local_string{cstr})}.allocator().init == "Default");
		static_assert(test(local_string{std::move(local_string{cstr2})}));
		static_assert(local_string{std::move(local_string{cstr, gstd::allocator_arguments, "Custom"})}.allocator().init == "Custom");
	}
	{
		constexpr local_string s{"Test"};
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
		static_assert(test(lstr_t{(char const *) "AAAAAAAAAA"}));
		static_assert(test(lstr_t{"AAAAAAAAAA"}));
		static_assert(test(sstr_t{10, 'A'}));
		static_assert(test(sstr_t{lstr_t{10, 'A'}}));
		static_assert(test(sstr_t{std::move(sstr_t{10, 'A'})}));
		static_assert(test(sstr_t{"AAAAAAAAAA", 10}));
		static_assert(test(sstr_t{(char const *) "AAAAAAAAAA"}));
		static_assert(test(sstr_t{"AAAAAAAAAA"}));
		constexpr local_string<256, custom_allocator> cstr{10, 'A'};
		constexpr local_string<256, custom_allocator> cstr2{10, 'A', gstd::allocator_arguments, "Custom"};
		constexpr local_string cstr3{cstr2};
		constexpr local_string cstr4{cstr2, gstd::allocator_arguments, cstr2.allocator()};
		constexpr local_string cstr5{cstr2, gstd::allocator_arguments, "Custom 2"};
		static_assert(test(cstr));
		static_assert(test(cstr2));
		static_assert(test(cstr3));
		static_assert(test(cstr4));
		static_assert(test(cstr5));
		static_assert(cstr.allocator().init == "Default");
		static_assert(cstr2.allocator().init == "Custom");
		static_assert(cstr3.allocator().init == "Default");
		static_assert(cstr4.allocator().init == "Custom");
		static_assert(cstr5.allocator().init == "Custom 2");
		static_assert(test(local_string{std::move(local_string{cstr})}));
		static_assert(local_string{std::move(local_string{cstr})}.allocator().init == "Default");
		static_assert(test(local_string{std::move(local_string{cstr2})}));
		static_assert(local_string{std::move(local_string{cstr, gstd::allocator_arguments, "Custom"})}.allocator().init == "Custom");
		static_assert(test(lstr_t{std::move(local_string{cstr})}));
		static_assert(test(sstr_t{std::move(local_string{cstr})}));
	}
}

consteval void assignments() noexcept {
	using gstd::local_string;
	using lstr_t = local_string<256>;
	using sstr_t = local_string<1>;
	lstr_t lstr, lstr1{"Long  1"}, lstr2{"Long  2"};
	sstr_t sstr, sstr1{"Short 1"}, sstr2{"Short 2"};
	local_string<256, custom_allocator> lcstr{"Custom", gstd::allocator_arguments, "Custom"};
	local_string<  1, custom_allocator> scstr{"Custom", gstd::allocator_arguments, "Custom"};
	assert(&(lstr = lstr1) == &lstr && lstr == "Long  1" && lstr1 == "Long  1" && lstr2 == "Long  2");
	assert(&(lstr = lstr2) == &lstr && lstr == "Long  2" && lstr1 == "Long  1" && lstr2 == "Long  2");
	assert(&(sstr = sstr1) == &sstr && sstr == "Short 1" && sstr1 == "Short 1" && sstr2 == "Short 2");
	assert(&(sstr = sstr2) == &sstr && sstr == "Short 2" && sstr1 == "Short 1" && sstr2 == "Short 2");
	assert(&(lstr = std::move(lstr1)) == &lstr && lstr == "Long  1" && lstr1 == "" && lstr2 == "Long  2");
	assert(&(sstr = std::move(sstr1)) == &sstr && sstr == "Short 1" && sstr1 == "" && sstr2 == "Short 2");
	assert((lstr = lcstr) == "Custom" && lcstr == "Custom");
	assert((sstr = scstr) == "Custom" && scstr == "Custom");
	assert((clear(lstr), lstr == ""));
	assert((clear(sstr), sstr == ""));
	assert((lstr = std::move(lcstr)) == "Custom" && lcstr == "Custom");
	assert((sstr = std::move(scstr)) == "Custom" && scstr == "Custom");
	assert((local_string{std::move(lcstr)}, lcstr == ""));
	assert((local_string{std::move(scstr)}, scstr == ""));
}

int main() {
	constructors();
	assignments();
}
