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

	constexpr void reallocate(char *& op, size_t oc, size_t nc) noexcept {
		allocations.emplace_back(-static_cast<int>(oc));
		allocations.emplace_back(static_cast<int>(nc));
		gstd::string::allocator::reallocate(op, oc, nc);
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
		static_assert(test(lstr_t{"AAAAAAAAAA"}));
		static_assert(test(sstr_t{10, 'A'}));
		static_assert(test(sstr_t{lstr_t{10, 'A'}}));
		static_assert(test(sstr_t{std::move(sstr_t{10, 'A'})}));
		static_assert(test(sstr_t{"AAAAAAAAAA", 10}));
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
	char       mc{'M'};
	char       cc{'C'};
	char       mbuf[]{"Mutable"};
	char const cbuf[]{"Constant"};
	unsigned char       uc{10};
	unsigned short      us{uc};
	unsigned int        ui{uc};
	unsigned long       ul{uc};
	unsigned long long ull{uc};
	sstr_t{10,  'A'};
	sstr_t{uc,  'A'};
	sstr_t{us,  'A'};
	sstr_t{ui,  'A'};
	sstr_t{ul,  'A'};
	sstr_t{ull, 'A'};
	sstr_t{10,  mc};
	sstr_t{uc,  mc};
	sstr_t{us,  mc};
	sstr_t{ui,  mc};
	sstr_t{ul,  mc};
	sstr_t{ull, mc};
	sstr_t{10,  cc};
	sstr_t{uc,  cc};
	sstr_t{us,  cc};
	sstr_t{ui,  cc};
	sstr_t{ul,  cc};
	sstr_t{ull, cc};
	local_string<1, custom_allocator>{10,  'A', gstd::allocator_arguments, "Custom"};
	local_string<1, custom_allocator>{uc,  'A', gstd::allocator_arguments, "Custom"};
	local_string<1, custom_allocator>{us,  'A', gstd::allocator_arguments, "Custom"};
	local_string<1, custom_allocator>{ui,  'A', gstd::allocator_arguments, "Custom"};
	local_string<1, custom_allocator>{ul,  'A', gstd::allocator_arguments, "Custom"};
	local_string<1, custom_allocator>{ull, 'A', gstd::allocator_arguments, "Custom"};
	local_string<1, custom_allocator>{10,  mc, gstd::allocator_arguments, "Custom"};
	local_string<1, custom_allocator>{uc,  mc, gstd::allocator_arguments, "Custom"};
	local_string<1, custom_allocator>{us,  mc, gstd::allocator_arguments, "Custom"};
	local_string<1, custom_allocator>{ui,  mc, gstd::allocator_arguments, "Custom"};
	local_string<1, custom_allocator>{ul,  mc, gstd::allocator_arguments, "Custom"};
	local_string<1, custom_allocator>{ull, mc, gstd::allocator_arguments, "Custom"};
	local_string<1, custom_allocator>{10,  cc, gstd::allocator_arguments, "Custom"};
	local_string<1, custom_allocator>{uc,  cc, gstd::allocator_arguments, "Custom"};
	local_string<1, custom_allocator>{us,  cc, gstd::allocator_arguments, "Custom"};
	local_string<1, custom_allocator>{ui,  cc, gstd::allocator_arguments, "Custom"};
	local_string<1, custom_allocator>{ul,  cc, gstd::allocator_arguments, "Custom"};
	local_string<1, custom_allocator>{ull, cc, gstd::allocator_arguments, "Custom"};
	local_string{"Mutable"};
	local_string{"Constant"};
	local_string{mbuf};
	local_string{cbuf};
	sstr_t{(char       *) mbuf};
	sstr_t{(char const *) cbuf};
	local_string<1, custom_allocator>{"Mutable", gstd::allocator_arguments, "Custom"};
	local_string<1, custom_allocator>{"Constant", gstd::allocator_arguments, "Custom"};
	local_string<1, custom_allocator>{(char const *) "Mutable", gstd::allocator_arguments, "Custom"};
	local_string<1, custom_allocator>{(char       *) "Constant", gstd::allocator_arguments, "Custom"};
	sstr_t{"Mutable", 7};
	sstr_t{"Constant", 8};
	sstr_t{mbuf, 7};
	sstr_t{cbuf, 8};
	local_string<1, custom_allocator>{"Mutable", 7, gstd::allocator_arguments, "Custom"};
	local_string<1, custom_allocator>{"Constant", 8, gstd::allocator_arguments, "Custom"};
	sstr_t{std::string_view{"Test"}};
	local_string<1, custom_allocator>{std::string_view{"Test"}, gstd::allocator_arguments, "Custom"};
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
	assert((lstr = "Array") == "Array");
	assert((sstr = "Array") == "Array");
	assert((lstr = (char const *) "Pointer") == (char const *) "Pointer");
	assert((sstr = (char const *) "Pointer") == (char const *) "Pointer");
	assert((lstr = 'A') == "A");
	assert((sstr = 'A') == "A");
}

consteval void memory() noexcept {
	gstd::local_string<1, custom_allocator> str;
	auto && actual = str.allocator().allocations;
	std::vector<int> expected;
	auto assert_allocs = [&](size_t cap) { assert(str.capacity() == cap && actual == expected); };
	assert_allocs(0);
	str.reserve(10); expected.emplace_back(11);
	assert_allocs(10);
	str.reserve(0);
	assert_allocs(10);
	assert(str.shrink_to_fit()); expected.emplace_back(-11);
	assert_allocs(0);
	str.reserve(10); expected.emplace_back(11);
	assert_allocs(10);
	str.reserve(20); expected.emplace_back(-11); expected.emplace_back(21);
	assert_allocs(20);
	str.size(10);
	assert(!str.shrink_to_fit());
	assert_allocs(20);
}

consteval void appending() noexcept {
	using gstd::local_string;
	local_string str{"A"};
	str += "B";
	assert(str == "AB");
	str += (char const *) "C";
	assert(str == "ABC");
	str += local_string{"D"};
	assert(str == "ABCD");
}

consteval void stack() noexcept {
	gstd::local_string<10> str;
	for(auto c : {'A', 'B', 'C', 'D'})
		push_back(str, c);
	assert(str == "ABCD");
	for(auto c : {'D', 'C', 'B', 'A'})
		assert(pop_back(str) == c);
	str = "XZYX";
	for(auto c : {'X', 'Y', 'Z'})
		assert(pop_back(str) == c);
	for(auto c : {'A', 'B', 'C', 'D'})
		str += c;
	assert(str == "XABCD");
	for(auto c : {'D', 'C', 'B', 'A'})
		assert(pop_back(str) == c);
	assert(str == "X");
}

consteval void insertions() noexcept {
	gstd::local_string<256> str;
	insert(str, 0, "ABCD");
	assert(str == "ABCD");
	insert(str, 0, 3, 'X');
	assert(str == "XXXABCD");
	insert(str, 5, std::string_view{"-"});
	assert(str == "XXXAB-CD");
	insert(str, str.size(), (char const *) "lol");
	assert(str == "XXXAB-CDlol");
}

consteval void erase() noexcept {
	gstd::local_string<256> str;
	str = "Testing LOL! xD";
	erase(str, 4, 3);
	assert(str == "Test LOL! xD");
	erase(str, 0, 5);
	assert(str == "LOL! xD");
	erase(str, 4);
	assert(str == "LOL!");
	erase(str, 3, 100);
	assert("LOL");
	erase(str, 0);
	assert(str == "");
}

consteval void searching() noexcept {
	using gstd::local_string, gstd::string::npos;
	constexpr local_string e{""};
	constexpr local_string str{"Lots of stuff to be found here."};
	assert(!starts_with(e, 'X'));
	assert(starts_with(str, 'L') && !starts_with(str, 'X'));
	assert(starts_with(e, "") && !starts_with(e, "Lot"));
	assert(starts_with(str, "Lot") && !starts_with(str, "Low") && !starts_with(str, "Lott"));
	assert(!ends_with(e, 'X'));
	assert(ends_with(str, '.') && !ends_with(str, 'X'));
	assert(ends_with(e, "") && !ends_with(e, "here."));
	assert(ends_with(str, "here.") && !ends_with(str, "hear.") && !ends_with(str, "heat."));
	assert(find(e, 'X') == npos);
	assert(find(str, 'L') == 0 && find(str, 'o') == 1 && find(str, 'o', 2) == 5 && find(str, '.') == str.size() - 1 && find(str, 'X') == npos);
	assert(find(e, "") == 0 && find(e, "X") == npos);
	assert(find(str, "Lot") == 0 && find(str, "Low") == npos);
	assert(rfind(e, 'X') == npos);
	assert(rfind(str, 'L') == 0 && rfind(str, 'o', 7) == 5 && rfind(str, 'o', 4) == 1 && rfind(str, '.') == str.size() - 1 && rfind(str, 'X') == npos);
	assert(rfind(e, "") == 0 && rfind(e, "X") == npos);
	assert(rfind(str, "Lot") == 0 && rfind(str, "Low") == npos);
}

int main() {
	constructors();
	assignments();
	memory();
	appending();
	stack();
	insertions();
	erase();
	searching();
}
