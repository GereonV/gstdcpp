#ifndef GSTD_ALLOCATION_HPP
#define GSTD_ALLOCATION_HPP

namespace gstd {

	using size_t = decltype(sizeof(nullptr));

	template<typename T>
	struct allocation_result {
		T * ptr;
		size_t count; // size if sizeof(T) == 0
	};

	class default_allocator {
	private:
		static allocation_result<void> do_allocation(size_t size) noexcept;
		static void * do_reallocation(void * ptr, size_t new_size) noexcept;
		static void do_deallocation(void * ptr) noexcept;
	public:
		template<typename T>
		static constexpr allocation_result<T> allocate(size_t count) noexcept {
			if consteval {
				return {new T[count], count};
			} else {
				auto [ptr, size] = do_allocation(count * sizeof(T));
				return {static_cast<T *>(ptr), size / sizeof(T)};
			}
		}

		template<typename T> // move-assign to unspecified should be equivalent to std::memcpy
		static constexpr T * reallocate(T * ptr, size_t old_count, size_t new_count) noexcept {
			if consteval {
				auto new_ptr = new T[new_count];
				auto copy_count = old_count > new_count ? new_count : old_count; // min
				for(size_t i{}; i < copy_count; ++i)
					new_ptr[i] = static_cast<T &&>(ptr[i]);
				delete[] ptr;
				return new_ptr;
			} else {
				auto new_ptr = do_reallocation(ptr, new_count * sizeof(T));
				return static_cast<T *>(new_ptr);
			}
		}

		template<typename T>
		static constexpr void deallocate(T * ptr) noexcept {
			if consteval {
				delete[] ptr;
			} else {
				do_deallocation(ptr);
			}
		}
	};

}

#endif // GSTD_ALLOCATION_HPP
