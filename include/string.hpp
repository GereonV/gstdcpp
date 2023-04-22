#ifndef GSTD_STRING_HPP
#define GSTD_STRING_HPP

#include "allocation.hpp"

// TODO iostream (<<, >>, getline)
// TODO std::hash

namespace gstd::string {

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

	class allocator {
	public:
		[[nodiscard]] static constexpr allocation_result<char> allocate(size_t count) {
			if consteval {
				return {new char[count], count};
			} else {
				auto [ptr, size] = do_allocation(count);
				if(!ptr) throw;
				return {static_cast<char *>(ptr), size};
			}
		}

		static constexpr void reallocate(char * & ptr, size_t old_count, size_t new_count) {
			if consteval {
				auto new_ptr = new char[new_count];
				auto copy_count = old_count > new_count ? new_count : old_count; // min
				for(size_t i{}; i < copy_count; ++i)
					new_ptr[i] = ptr[i];
				delete[] ptr;
				ptr = new_ptr;
			} else {
				auto new_ptr = do_reallocation(ptr, new_count);
				if(!new_ptr) throw;
				ptr = static_cast<char *>(new_ptr);
			}
		}

		static constexpr void deallocate(char * ptr, size_t) noexcept {
			if consteval {
				delete[] ptr;
			} else {
				do_deallocation(ptr);
			}
		}
	};

	inline constexpr auto npos = static_cast<size_t>(-1);

	// Allocator:
	// - gstd::allocation_result<char> allocate(size_t count)
	// - void reallocate(char * & old_ptr, size_t old_count, size_t new_count)
	// - void deallocate(char * ptr, size_t count) noexcept
	template<size_t N, typename Allocator = allocator>
	class local_string {
	static_assert(N); // sizeof("") == 1
	public:
		constexpr local_string() noexcept(noexcept(Allocator{}))
			: local_string{allocator_arguments} {}

		template<typename... T>
		constexpr local_string(allocator_arguments_marker, T &&... args) noexcept(noexcept(Allocator{static_cast<T &&>(args)...}))
			: local_string{0, [](char *) {}, static_cast<T &&>(args)...} {}

		constexpr local_string(local_string const & other)
			: local_string{other.begin(), other.size()} {}

		template<size_t M, typename A, typename... T>
		constexpr local_string(local_string<M, A> const & other, T &&... args)
			: local_string{other.begin(), other.size(), static_cast<T &&>(args)...} {}

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

		template<size_t M>
		constexpr local_string(char const (& source)[M]) noexcept(M <= N)
			: local_string{source, M - 1} {}

		template<typename... T>
		constexpr local_string(size_t count, char c, T &&... args)
			: local_string{count, [count, c](char * dst) {
				set(dst, count, c);
			}, static_cast<T &&>(args)...} {}

		template<typename... T>
		constexpr local_string(char const * source, size_t size, T &&... args)
			: local_string{size, [source, size](char * dst) {
				cpyfwd(dst, source, size);
			}, static_cast<T &&>(args)...} {}

		constexpr ~local_string() {
			_allocator.deallocate(_ptr, _capacity);
		}

		template<size_t M, typename A>
		constexpr local_string & operator=(local_string<M, A> const & rhs) {
			assign(*this, rhs.begin(), rhs.size());
			return *this;
		}
	
		constexpr local_string & operator=(local_string && rhs) noexcept(noexcept(_allocator = static_cast<Allocator &&>(_allocator))) {
			if(this == &rhs)
				return *this;
			_ptr = rhs._ptr;
			_size = rhs._size;
			_capacity = rhs._capacity;
			_allocator = std::move(rhs._allocator);
			rhs._ptr = nullptr;
			rhs._size = 0;
			rhs._capacity = N;
			if(!_ptr)
				cpyfwd(_buffer, rhs._buffer, _size + 1);
			return *this;
		}

		template<size_t M>
		constexpr local_string & operator=(char const (& source)[M]) noexcept(M <= N) {
			assign(*this, source, M - 1);
			return *this;
		}

		constexpr local_string & operator=(char c) {
			assign(*this, &c, 1);
			return *this;
		}

		[[nodiscard]] constexpr Allocator & allocator() noexcept {
			return _allocator;
		}

		[[nodiscard]] constexpr Allocator const & allocator() const noexcept {
			return _allocator;
		}

		[[nodiscard]] constexpr char & operator[](size_t pos) noexcept {
			return begin()[pos];
		}

		[[nodiscard]] constexpr char const & operator[](size_t pos) const noexcept {
			return begin()[pos];
		}

		[[nodiscard]] constexpr char * begin() noexcept {
			return _ptr ? _ptr : _buffer; // make the elvis operator standard already
		}

		[[nodiscard]] constexpr char const * begin() const noexcept {
			return _ptr ? _ptr : _buffer; // pretty please
		}

		[[nodiscard]] constexpr char * end() noexcept {
			return begin() + _size;
		}

		[[nodiscard]] constexpr char const * end() const noexcept {
			return begin() + _size;
		}

		// TODO rbegin() & rend()

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
				return begin();
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
				dst = src = begin() + pos;
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
			_ptr = _allocator.reallocate(_ptr, _capacity, new_capacity);
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

	template<size_t N, typename A, size_t M, typename B>
	constexpr bool operator==(local_string<N, A> const & lhs, local_string<M, B> const & rhs) noexcept {
		auto size = lhs.size();
		if(size != rhs.size())
			return false;
		auto a = lhs.begin(), b = rhs.begin(); 
		while(size--)
			if(a[size] != b[size])
				return false;
		return true;
	}

	template<size_t N, typename A, size_t M, typename B>
	constexpr auto operator<=>(local_string<N, A> const & lhs, local_string<M, B> const & rhs) noexcept {
		auto lsize = lhs.size(), rsize = rhs.size();
		auto min_size = lsize < rsize ? lsize : rsize;
		auto a = lhs.begin(), b = rhs.begin();
		for(size_t i{}; i < min_size; ++i)
			if(auto cmp = a[i] <=> b[i]; cmp != 0)
				return cmp;
		return lsize <=> rsize;
	}

	// TODO operator+
	// TODO operator*
	// TODO operator*=

	template<size_t N, typename A, size_t M, typename B>
	constexpr local_string<N, A> & operator+=(local_string<N, A> & lhs, local_string<M, B> const & rhs) noexcept {
		append(lhs, rhs.begin(), rhs.size());
		return lhs;
	}

	template<size_t N, typename A, size_t M>
	constexpr local_string<N, A> & operator+=(local_string<N, A> & lhs, char const (& rhs)[M]) noexcept {
		append(lhs, rhs, M - 1);
		return lhs;
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
	constexpr void insert(local_string<N, A> & s, size_t pos, size_t count, char c) noexcept {
		auto ptr = s.insert_unspecified(pos, count);
		set(ptr, count, c);
	}

	// pos ∈ [0, size()]
	template<size_t N, typename A>
	constexpr void insert(local_string<N, A> & s, size_t pos, char const * source, size_t size) noexcept {
		auto ptr = s.insert_unspecified(pos, size);
		cpyfwd(ptr, source, size);
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

	// TODO bool starts_with noexcept (char), (char const *, size_t), (char const *)
	// TODO bool ends_with noexcept (char), (char const *, size_t),
	// TODO size_t find(..., pos = 0) (char), (char const *, size_t), (char const *) // npos
	// TODO size_t rfind(..., pos = npos) (char), (char const *, size_t) // npos
}

namespace gstd {
	using string::local_string;
}

#endif // GSTD_STRING_HPP
