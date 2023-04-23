#ifndef GSTD_STRING_HPP
#define GSTD_STRING_HPP

#include "allocation.hpp"

#include <string_view>

// TODO iostream (<<, >>, getline)
// TODO numeric conversion

namespace gstd::string {
	namespace detail {
		template<typename, typename> inline constexpr auto is_same = false;
		template<typename T> inline constexpr auto is_same<T, T> = true;
	}

	// pray to the optimizer
	constexpr void cpyfwd(char * dst, char const * src, size_t amt) noexcept {
		for(auto end = src + amt; src != end; ++src, ++dst)
			*dst = *src;
	}

	// pray to the optimizer
	constexpr void cpybwd(char * dst, char const * src, size_t amt) noexcept {
		while(amt--)
			dst[amt] = src[amt];
	}

	// pray to the optimizer
	constexpr void set(char * dst, size_t amt, char c) noexcept {
		for(auto end = dst + amt; dst != end; ++dst)
			*dst = c;
	}

	inline constexpr auto npos = static_cast<size_t>(-1);

	// Allocator:
	// - gstd::allocation_result<char> allocate(size_t count)
	// - void reallocate(char * & old_ptr, size_t old_count, size_t new_count)
	// - void deallocate(char * ptr, size_t count) noexcept
	template<size_t N, typename Allocator = allocator>
	class local_string {
	static_assert(N); // sizeof("") == 1
	public:
		static constexpr auto npos = string::npos;

		constexpr local_string() noexcept(noexcept(Allocator{}))
			: local_string{allocator_arguments} {}

		constexpr local_string(local_string const & other)
			: local_string{other, allocator_arguments} {}

		constexpr local_string(local_string && other) noexcept(noexcept(Allocator{static_cast<Allocator &&>(_allocator)}))
			: _allocator{static_cast<Allocator &&>(other._allocator)},
			  _ptr{other._ptr}, _size{other._size}, _capacity{other._capacity} {
			if consteval { set(_buffer, N, 0); }
			if(_ptr)
				other._ptr = nullptr;
			else
				cpyfwd(_buffer, other._buffer, _size + 1);
			other._size = 0;
			other._capacity = N;
			other._buffer[0] = 0;
		}

		constexpr local_string(size_t count, char c)
			: local_string{count, c, allocator_arguments} {}

		constexpr explicit local_string(char const * source)
			: local_string{source, allocator_arguments} {}

		constexpr local_string(char const * source, size_t size)
			: local_string{source, size, allocator_arguments} {}

		template<size_t M>
		constexpr explicit(M > N) local_string(char const (& source)[M]) noexcept(M <= N && noexcept(Allocator{}))
			: local_string{source, allocator_arguments} {}

		constexpr explicit local_string(auto const & range)
			: local_string{range, allocator_arguments} {}

		template<typename... T>
		constexpr local_string(allocator_arguments_marker, T &&... args) noexcept(noexcept(Allocator{static_cast<T &&>(args)...}))
			: local_string{0, [](char *) {}, static_cast<T &&>(args)...} {}

		template<typename... T>
		constexpr local_string(size_t count, char c, allocator_arguments_marker, T &&... args)
			: local_string{count, [count, c](char * dst) {
				set(dst, count, c);
			  } , static_cast<T &&>(args)...} {}

		template<typename... T>
		constexpr local_string(char const * source, allocator_arguments_marker, T &&... args)
			: local_string{std::string_view{source}, allocator_arguments, static_cast<T &&>(args)...} {}

		template<typename... T>
		constexpr local_string(char const * source, size_t size, allocator_arguments_marker, T &&... args)
			: local_string{size, [source, size](char * dst) {
				cpyfwd(dst, source, size);
			  }, static_cast<T &&>(args)...} {}

		template<size_t M, typename... T>
		constexpr local_string(auto const & source, allocator_arguments_marker, T &&... args) noexcept(M <= N && noexcept(Allocator{static_cast<T &&>(args)...}))
			requires detail::is_same<decltype(source), char const (&)[M]>
			: local_string{source, M - 1, allocator_arguments, static_cast<T &&>(args)...} {}

		template<typename R, typename... T>
		constexpr local_string(R const & range, allocator_arguments_marker, T &&... args)
			requires requires(R const & r) { r.data(); r.size(); }
			: local_string{range.data(), range.size(), allocator_arguments, static_cast<T &&>(args)...} {}

		constexpr ~local_string() {
			_allocator.deallocate(_ptr, _capacity);
		}

		constexpr local_string & operator=(local_string const & rhs) {
			assign(*this, rhs);
			return *this;
		}
	
		constexpr local_string & operator=(local_string && rhs) noexcept(noexcept(_allocator = static_cast<Allocator &&>(_allocator))) {
			if(this == &rhs)
				return *this;
			_allocator.deallocate(_ptr, _capacity);
			_ptr = rhs._ptr;
			_size = rhs._size;
			_capacity = rhs._capacity;
			_allocator = static_cast<Allocator &&>(rhs._allocator);
			rhs._ptr = nullptr;
			rhs._size = 0;
			rhs._capacity = N;
			if(!_ptr)
				cpyfwd(_buffer, rhs._buffer, _size + 1);
			return *this;
		}

		constexpr local_string & operator=(char c) {
			assign(*this, 1, c);
			return *this;
		}

		template<size_t M>
		constexpr local_string & operator=(char const (& source)[M]) noexcept(M <= N) {
			assign(*this, source);
			return *this;
		}

		constexpr local_string & operator=(auto const & range) {
			assign(*this, range);
			return *this;
		}

		[[nodiscard]] constexpr operator std::string_view() const noexcept {
			return {data(), _size};
		}

		[[nodiscard]] constexpr Allocator & allocator() noexcept {
			return _allocator;
		}

		[[nodiscard]] constexpr Allocator const & allocator() const noexcept {
			return _allocator;
		}

		[[nodiscard]] constexpr char & operator[](size_t pos) noexcept {
			return data()[pos];
		}

		[[nodiscard]] constexpr char const & operator[](size_t pos) const noexcept {
			return data()[pos];
		}

		[[nodiscard]] constexpr char * data() noexcept {
			return _ptr ? _ptr : _buffer; // make the elvis operator standard already
		}

		[[nodiscard]] constexpr char const * data() const noexcept {
			return _ptr ? _ptr : _buffer; // pretty please
		}

		[[nodiscard]] constexpr size_t size() const noexcept {
			return _size;
		}

		constexpr void size(size_t new_size) noexcept {
			_size = new_size;
		}

		[[nodiscard]] constexpr size_t capacity() const noexcept {
			return _capacity - 1; // usable capacity without 0 terminator
		}

		constexpr char * reserve(size_t new_capacity) {
			if(new_capacity < _capacity)
				return data();
			++new_capacity;
			if(!_ptr) {
				allocate(new_capacity);
				cpyfwd(_ptr, _buffer, _size + 1);
			} else {
				reallocate(new_capacity);
			}
			return _ptr;
		}

		// pos ∈ [0, size()]
		[[nodiscard]] constexpr char * insert_unspecified(size_t pos, size_t count) {
			auto new_size = _size + count;
			char * dst, * src;
			if(new_size < _capacity) {
				dst = src = data() + pos;
			} else if(!_ptr) {
				allocate(new_size + 1);
				cpyfwd(_ptr, _buffer, pos);
				dst = _ptr + pos;
				src = _buffer + pos;
			} else {
				reallocate(new_size + 1);
				dst = src = _ptr + pos;
			}
			cpybwd(dst + count, src, _size - pos + 1);
			_size = new_size;
			return dst;
		}

		constexpr bool shrink_to_fit() noexcept {
			if(_size >= N)
				return false;
			cpyfwd(_buffer, _ptr, _size + 1);
			_allocator.deallocate(_ptr, _capacity);
			_ptr = nullptr;
			_capacity = N;
			return true;
		}
	private:
		constexpr void allocate(size_t size) noexcept {
			auto [ptr, capacity] = _allocator.allocate(size);
			_ptr = ptr;
			_capacity = capacity;
		}

		constexpr void reallocate(size_t new_capacity) noexcept {
			_allocator.reallocate(_ptr, _capacity, new_capacity);
			_capacity = new_capacity;
		}

		template<typename... T>
		constexpr local_string(size_t count, auto && f, T &&... args)
			: _allocator{static_cast<T &&>(args)...}, _size{count} {
			if consteval { set(_buffer, N, 0); }
			char * ptr;
			if(_size >= N) {
				allocate(_size + 1);
				ptr = _ptr;
			} else {
				_ptr = nullptr;
				_capacity = N;
				ptr = _buffer;
			}
			f(ptr);
			ptr[_size] = 0;
		}
	private:
		[[no_unique_address]] Allocator _allocator;
		char * _ptr; // nullptr or heap
		size_t _size; // always accurate
		size_t _capacity; // always accurate
		char _buffer[N]; // unused if non-local
	};

	template<size_t N>
	local_string(char const (&)[N]) -> local_string<N>;

	template<size_t N, typename A>
	local_string(local_string<N, A> const &, allocator_arguments_marker, auto...) -> local_string<N, A>;

	template<size_t N, typename A>
	[[nodiscard]] constexpr char * begin(local_string<N, A> & s) noexcept {
		return s.data();
	}

	template<size_t N, typename A>
	[[nodiscard]] constexpr char const * begin(local_string<N, A> const & s) noexcept {
		return s.data();
	}

	template<size_t N, typename A>
	[[nodiscard]] constexpr char * end(local_string<N, A> & s) noexcept {
		return s.data() + s.size();
	}

	template<size_t N, typename A>
	[[nodiscard]] constexpr char const * end(local_string<N, A> const & s) noexcept {
		return s.data() + s.size();
	}

	// TODO rbegin() & rend()

	template<size_t N, typename A>
	constexpr bool operator==(local_string<N, A> const & lhs, std::string_view rhs) noexcept {
		return static_cast<std::string_view>(lhs) == rhs;
	}

	template<size_t N, typename A, size_t M>
	constexpr bool operator==(local_string<N, A> const & lhs, char const (& rhs)[M]) noexcept {
		return lhs == std::string_view{rhs, M - 1};
	}

	template<size_t N, typename A>
	constexpr bool operator==(local_string<N, A> const & lhs, auto const & rhs) noexcept {
		return lhs == std::string_view{rhs.data(), rhs.size()};
	}

	template<size_t N, typename A>
	constexpr bool operator<=>(local_string<N, A> const & lhs, std::string_view rhs) noexcept {
		return static_cast<std::string_view>(lhs) == rhs;
	}

	template<size_t N, typename A, size_t M>
	constexpr bool operator<=>(local_string<N, A> const & lhs, char const (& rhs)[M]) noexcept {
		return lhs <=> std::string_view{rhs, M - 1};
	}

	template<size_t N, typename A>
	constexpr auto operator<=>(local_string<N, A> const & lhs, auto const & rhs) noexcept {
		return lhs <=> std::string_view{rhs.data(), rhs.size()};
	}

	// TODO operator*
	// TODO operator*=

	template<size_t N, typename A, size_t M>
	constexpr local_string<N, A> & operator+=(local_string<N, A> & lhs, char const (& rhs)[M]) {
		append(lhs, rhs, M - 1);
		return lhs;
	}

	template<size_t N, typename A>
	constexpr local_string<N, A> & operator+=(local_string<N, A> & lhs, auto const & range) {
		append(lhs, range.data(), range.size());
		return lhs;
	}

	template<size_t N, typename A, size_t M>
	constexpr local_string<N, A> operator+(local_string<N, A> const & lhs, char const (& rhs)[M]) {
		auto cpy = lhs;
		return cpy += rhs;
	}

	template<size_t N, typename A>
	constexpr local_string<N, A> operator+(local_string<N, A> const & lhs, auto const & range) {
		auto cpy = lhs;
		return cpy += range;
	}

	template<size_t N, typename A, size_t M>
	constexpr local_string<N, A> operator+(local_string<N, A> && lhs, char const (& rhs)[M]) {
		lhs += rhs;
		return static_cast<local_string<N, A> &&>(lhs);
	}

	template<size_t N, typename A>
	constexpr local_string<N, A> operator+(local_string<N, A> && lhs, auto const & range) {
		lhs += range;
		return static_cast<local_string<N, A> &&>(lhs);
	}

	template<size_t N, typename A>
	constexpr void push_back(local_string<N, A> & s, char c) {
		auto size = s.size();
		auto new_size = size + 1;
		auto ptr = s.reserve(s.capacity() * 2);
		s.size(new_size);
		ptr[size] = c;
		ptr[new_size] = 0;
	}

	template<size_t N, typename A>
	constexpr char pop_back(local_string<N, A> & s) noexcept {
		auto new_size = s.size() - 1;
		auto ptr = s.begin();
		auto c = ptr[new_size];
		s.size(new_size);
		ptr[new_size] = 0;
		return c;
	}

	template<size_t N, typename A>
	constexpr void clear(local_string<N, A> & s) noexcept {
		s.size(0);
		s[0] = 0;
	}

	// pos ∈ [0, size()]
	template<size_t N, typename A>
	constexpr void insert(local_string<N, A> & s, size_t pos, size_t count, char c) {
		auto ptr = s.insert_unspecified(pos, count);
		set(ptr, count, c);
	}

	// pos ∈ [0, size()]
	template<size_t N, typename A>
	constexpr void insert(local_string<N, A> & s, size_t pos, char const * source, size_t size) {
		auto ptr = s.insert_unspecified(pos, size);
		cpyfwd(ptr, source, size);
	}

	// pos ∈ [0, size()]
	template<size_t N, typename A, size_t M>
	constexpr void insert(local_string<N, A> & s, size_t pos, char const (& source)[M]) {
		insert(s, source, M - 1);
	}

	// pos ∈ [0, size()]
	template<size_t N, typename A>
	constexpr void insert(local_string<N, A> & s, size_t pos, auto const & range) {
		insert(s, range.data(), range.size());
	}

	template<size_t N, typename A>
	[[nodiscard]] constexpr char * append_unspecified(local_string<N, A> & s, size_t count) {
		auto size = s.size();
		auto new_size = size + count;
		auto ptr = s.reserve(new_size);
		s.size(new_size);
		ptr[new_size] = 0;
		return ptr + size;
	}

	template<size_t N, typename A, size_t M>
	constexpr void append(local_string<N, A> & s, size_t count, char c) {
		auto ptr = append_unspecified(s, count);
		set(ptr, count, c);
	}

	template<size_t N, typename A>
	constexpr void append(local_string<N, A> & s, char const * source, size_t size) {
		auto ptr = append_unspecified(s, size);
		cpyfwd(ptr, source, size);
	}

	template<size_t N, typename A, size_t M>
	constexpr void append(local_string<N, A> & s, char const (& source)[M]) {
		append(s, source, M - 1);
	}

	template<size_t N, typename A>
	constexpr void append(local_string<N, A> & s, auto const & range) {
		append(s, range.data(), range.size());
	}

	template<size_t N, typename A>
	constexpr void assign(local_string<N, A> & s, size_t count, char c) {
		auto ptr = s.reserve(count);
		s.size(count);
		set(ptr, count, c);
		ptr[count] = 0;
	}

	template<size_t N, typename A>
	constexpr void assign(local_string<N, A> & s, char const * source, size_t size) {
		auto ptr = s.reserve(size);
		s.size(size);
		cpyfwd(ptr, source, size);
		ptr[size] = 0;
	}

	template<size_t N, typename A, size_t M>
	constexpr void assign(local_string<N, A> & s, char const (& source)[M]) {
		assign(s, source, M - 1);
	}

	template<size_t N, typename A>
	constexpr void assign(local_string<N, A> & s, auto const & range) {
		assign(s, range.data(), range.size());
	}

	// pos ∈ [0, size()]
	template<size_t N, typename A>
	constexpr void erase(local_string<N, A> & s, size_t pos, size_t count = npos) noexcept {
		auto size = s.size();
		auto ptr = s.begin() + pos;
		if(count == npos || pos + count >= size) {
			s.size(pos);
			*ptr = 0;
		} else {
			size -= count;
			s.size(size);
			cpyfwd(ptr, ptr + count, size - pos + 1);
		}
	}

	template<size_t N, typename A>
	constexpr bool starts_with(local_string<N, A> const & s, char c) noexcept {
		return s[0] == c; // ok because of null terminator
	}

	template<size_t N, typename A>
	constexpr bool starts_with(local_string<N, A> const & s, char const * t) noexcept {
		return static_cast<std::string_view>(s).starts_with(t);
	}

	template<size_t N, typename A>
	constexpr bool starts_with(local_string<N, A> const & s, char const * t, size_t size) noexcept {
		return static_cast<std::string_view>(s).starts_with({t, size});
	}

	template<size_t N, typename A, size_t M>
	constexpr bool starts_with(local_string<N, A> const & s, char const (& t)[M]) noexcept {
		return starts_with(s, t, M - 1);
	}

	template<size_t N, typename A>
	constexpr bool starts_with(local_string<N, A> const & s, auto const & range) noexcept {
		return starts_with(s, range.data(), range.size());
	}

	template<size_t N, typename A>
	constexpr bool ends_with(local_string<N, A> const & s, char c) noexcept {
		return s.size() && s[s.size() - 1] == c;
	}

	template<size_t N, typename A>
	constexpr bool ends_with(local_string<N, A> const & s, char const * t) noexcept {
		return static_cast<std::string_view>(s).ends_with(t);
	}

	template<size_t N, typename A>
	constexpr bool ends_with(local_string<N, A> const & s, char const * t, size_t size) noexcept {
		return static_cast<std::string_view>(s).ends_with({t, size});
	}

	template<size_t N, typename A, size_t M>
	constexpr bool ends_with(local_string<N, A> const & s, char const (& t)[M]) noexcept {
		return ends_with(s, t, M - 1);
	}

	template<size_t N, typename A>
	constexpr bool ends_with(local_string<N, A> const & s, auto const & range) noexcept {
		return ends_with(s, range.data(), range.size());
	}

	template<size_t N, typename A>
	constexpr size_t find(local_string<N, A> const & s, char c, size_t pos = 0) noexcept {
		return static_cast<std::string_view>(s).find(c, pos);
	}

	template<size_t N, typename A>
	constexpr size_t find(local_string<N, A> const & s, char const * t, size_t pos = 0) noexcept {
		return static_cast<std::string_view>(s).find(t, pos);
	}

	template<size_t N, typename A>
	constexpr size_t find(local_string<N, A> const & s, char const * t, size_t size, size_t pos = 0) noexcept {
		return static_cast<std::string_view>(s).find({t, size}, pos);
	}

	template<size_t N, typename A, size_t M>
	constexpr size_t find(local_string<N, A> const & s, char const (& t)[M], size_t pos = 0) noexcept {
		return find(s, t, M - 1, pos);
	}

	template<size_t N, typename A>
	constexpr size_t find(local_string<N, A> const & s, auto const & range, size_t pos = 0) noexcept {
		return find(s, range.data(), range.size(), pos);
	}

	template<size_t N, typename A>
	constexpr size_t rfind(local_string<N, A> const & s, char c, size_t pos = npos) noexcept {
		return static_cast<std::string_view>(s).rfind(c, pos);
	}

	template<size_t N, typename A>
	constexpr size_t rfind(local_string<N, A> const & s, char const * t, size_t pos = npos) noexcept {
		return static_cast<std::string_view>(s).rfind(t, pos);
	}

	template<size_t N, typename A>
	constexpr size_t rfind(local_string<N, A> const & s, char const * t, size_t size, size_t pos = npos) noexcept {
		return static_cast<std::string_view>(s).rfind({t, size}, pos);
	}

	template<size_t N, typename A, size_t M>
	constexpr size_t rfind(local_string<N, A> const & s, char const (& t)[M], size_t pos = npos) noexcept {
		return rfind(s, t, M - 1, pos);
	}

	template<size_t N, typename A>
	constexpr size_t rfind(local_string<N, A> const & s, auto const & range, size_t pos = npos) noexcept {
		return rfind(s, range.data(), range.size(), pos);
	}

	template<size_t N, typename A>
	std::ostream & operator<<(std::ostream & os, local_string<N, A> const & s) {
		return os << s;
	}

	// TODO
	// template<size_t N, typename A>
	// std::istream & operator>>(std::istream & is, local_string<N, A> & s) {
	// 	return is;
	// }

	// TODO getline()
}

namespace std {
	template<typename T> struct hash;
	template<size_t N, typename A> struct hash<gstd::string::local_string<N, A>> {
		constexpr size_t operator()(gstd::string::local_string<N, A> const & s) const noexcept {
			return std::hash<std::string_view>{}(s);
#if 0
			// xor-version of djb2 (Dan Bernstein)
			size_t hash = 5381;
			for(auto c : s)
				hash = hash * 33 ^ c;
			return hash;
#endif
		}
	};
}

namespace gstd {
	using string::local_string;
}

#endif // GSTD_STRING_HPP
