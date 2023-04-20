#ifndef GSTD_STRING_HPP
#define GSTD_STRING_HPP

#include "allocation.hpp"

namespace gstd {

	constexpr size_t strlen(char const * s) noexcept {
		size_t len{};
		while(*s++) ++len;
		return len;
	}

	constexpr void strcpy(char * dst, char const * src, size_t amt) noexcept {
		while(amt--)
			dst[amt] = src[amt];
	}

	template<size_t N, typename Allocator = default_allocator>
	class local_string {
	static_assert(N);
	public:
		static constexpr auto npos = static_cast<size_t>(-1);

		constexpr local_string() noexcept(noexcept(Allocator{Allocator{}}))
			: local_string{Allocator{}} {}

		constexpr explicit local_string(Allocator && allocator) noexcept(noexcept(Allocator{static_cast<Allocator &&>(allocator)}))
			: _allocator{static_cast<Allocator &&>(allocator)}, _ptr{}, _size{}, _capacity{N} {
			_buffer[0] = 0;
		}

		template<size_t M, typename A>
		constexpr local_string(local_string<M, A> const & other, Allocator && allocator = Allocator{}) noexcept(noexcept(Allocator{static_cast<Allocator &&>(allocator)}))
			: local_string{other.begin(), other._size, static_cast<Allocator &&>(allocator)} {}

		constexpr local_string(local_string && other) noexcept(noexcept(Allocator{static_cast<Allocator &&>(other._allocator)}))
			: _allocator{static_cast<Allocator &&>(other._allocator)}, _ptr{other._ptr}, _size{other._size}, _capacity{other._capacity} {
			other._size = 0;
			other._capacity = N;
			other._buffer[0] = 0;
			if(_ptr) {
				other._ptr = nullptr;
				return;
			}
			strcpy(_buffer, other._buffer, _size + 1);
		}

		constexpr local_string(size_t count, char c, Allocator && allocator = Allocator{}) noexcept(noexcept(Allocator{static_cast<Allocator &&>(allocator)}))
			: local_string{count, static_cast<Allocator &&>(allocator), [count, c](char * dst) mutable {
				while(count--)
					dst[count] = c;
			}} {}

		constexpr local_string(char const * source, size_t size, Allocator && allocator = Allocator{}) noexcept(noexcept(Allocator{static_cast<Allocator &&>(allocator)}))
			: local_string{size, static_cast<Allocator &&>(allocator), [source, size](char * dst) {
				strcpy(dst, source, size);
			}} {}

		constexpr local_string(char const * source, Allocator && allocator = Allocator{}) noexcept(noexcept(Allocator{static_cast<Allocator &&>(allocator)}))
			: local_string{source, strlen(source), static_cast<Allocator &&>(allocator)} {}

		// TODO versions without allocator

		constexpr ~local_string() {
			_allocator.deallocate(_ptr);
		}

		constexpr char & operator[](size_t pos) noexcept {
			return begin()[pos];
		}

		constexpr char const & operator[](size_t pos) const noexcept {
			return begin()[pos];
		}

		constexpr char * begin() noexcept {
			return _ptr ? _ptr : _buffer; // make the elvis operator standard already
		}

		constexpr char const * begin() const noexcept {
			return _ptr ? _ptr : _buffer; // pretty please
		}

		constexpr char * end() noexcept {
			return begin() + _size;
		}

		constexpr char const * end() const noexcept {
			return begin() + _size;
		}

		// TODO rbegin() & rend()

		constexpr size_t size() noexcept {
			return _size;
		}

		constexpr void reserve(size_t new_capacity) noexcept {
			if(new_capacity < _capacity)
				return;
			++new_capacity;
			if(!_ptr) {
				allocate(new_capacity);
				strcpy(_ptr, _buffer, _size + 1);
			} else {
				reallocate(new_capacity);
			}
		}

		constexpr size_t capacity() const noexcept {
			return _capacity - 1; // usable capacity without 0 terminator
		}

		constexpr bool shrink_to_fit() noexcept {
			if(_size >= N)
				return false;
			strcpy(_buffer, _ptr, size + 1);
			_allocator.deallocate(_ptr);
			_ptr = nullptr;
			_capacity = N;
			return true;
		}

		constexpr void clear() noexcept {
			_size = 0;
			*begin() = 0;
		}

		// pos ∈ [0, size()]
		constexpr void insert(size_t pos, size_t count, char c) noexcept {
			insert_with(pos, count, [count, c](char * dst) mutable {
				while(count--)
					dst[count] = c;
			});
		}

		// pos ∈ [0, size()]
		constexpr void insert(size_t pos, char const * source, size_t size) noexcept {
			insert_with(pos, size, [source, size](char * dst) {
				strcpy(dst, source, size);
			});
		}

		// pos ∈ [0, size())
		constexpr void erase(size_t pos, size_t count = npos) noexcept {
			auto ptr = begin() + pos;
			if(count == npos || pos + count >= _size) {
				*ptr = 0;
				_size = pos;
				return;
			}
			strcpy(ptr, ptr + count, _size - pos - count + 1);
		}

		constexpr void push_back(char c) noexcept {
			auto old_size = _size;
			char * ptr;
			if(++_size == _capacity) {
				auto new_capacity = _capacity * 2;
				if(!_ptr) {
					allocate(new_capacity);
					strcpy(_ptr, _buffer, old_size);
				} else {
					reallocate(new_capacity);
				}
				ptr = _ptr;
			} else {
				ptr = begin();
			}
			ptr[old_size] = c;
			ptr[_size] = 0;
		}

		constexpr char pop_back() noexcept {
			auto ptr = begin() + --_size;
			auto c = *ptr;
			*ptr = 0;
			return c;
		}

		constexpr void append_from_capacity(size_t count) noexcept {
			append_with(count, [](char *) {});
		}

		constexpr void append(size_t count, char c) noexcept {
			append_with(count, [count, c](char * dst) mutable {
				while(count--)
					dst[count] = c;
			});
		}

		constexpr void append(char const * source, size_t size) noexcept {
			append_with(size, [source, size](char * dst) {
				strcpy(dst, source, size);
			});
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

		constexpr local_string(size_t count, Allocator && allocator, auto && f) noexcept(noexcept(Allocator{static_cast<Allocator &&>(allocator)}))
			: _allocator{static_cast<Allocator &&>(allocator)}, _size{count} {
			char * ptr;
			if(_size >= N) {
				allocate(_size + 1);
				ptr = _ptr;
			} else {
				_ptr = nullptr;
				_capacity = nullptr;
				ptr = _buffer;
			}
			f(ptr);
			ptr[_size] = 0;
		}

		constexpr void insert_with(size_t pos, size_t count, auto && f) noexcept {
			auto old_size = _size;
			_size += count;
			char * dst, src;
			if(_size < _capacity) {
				dst = src = begin() + pos;
			} else if(!_ptr) {
				allocate(_size + 1);
				strcpy(_ptr, _buffer, pos);
				dst = _ptr + pos;
				src = _buffer + pos;
			} else {
				reallocate(_size + 1);
				dst = src = _ptr + pos;
			}
			strcpy(dst + count, src, old_size - pos + 1); // copy within ok since copied back to front
			f(dst);
		}

		constexpr void append_with(size_t count, auto && f) noexcept {
			auto old_size = _size;
			_size += count;
			char * dst;
			if(_size >= _capacity) {
				auto new_capacity = _size + 1;
				if(!_ptr)
					allocate(new_capacity);
				else
					reallocate(new_capacity);
				dst = _ptr;
			} else {
				dst = begin();
			}
			f(dst + old_size);
			dst[_size] = 0;
		}
	private:
		[[no_unique_address]] Allocator _allocator;
		char * _ptr; // nullptr or heap
		size_t _size; // always accurate
		size_t _capacity; // always accurate
		char _buffer[N]; // unused if non-local
	};

	template<size_t N, typename A, size_t M, typename B>
	constexpr local_string<N, A> & operator+=(local_string<N, A> & lhs, local_string<M, B> const & rhs) noexcept {
		lhs.append(rhs.begin(), rhs.size());
		return lhs;
	}

	template<size_t N, typename A>
	constexpr local_string<N, A> & operator+=(local_string<N, A> & lhs, char const * rhs) noexcept {
		lhs.append(rhs, strlen(rhs));
		return lhs;
	}
}

#endif // GSTD_STRING_HPP
