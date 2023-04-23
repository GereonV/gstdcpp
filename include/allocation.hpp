#ifndef GSTD_ALLOCATION_HPP
#define GSTD_ALLOCATION_HPP

namespace gstd {
	using size_t = decltype(sizeof(nullptr));

	template<typename T>
	struct allocation_result {
		T * ptr;
		size_t count; // size if sizeof(T) == 0
	};

	[[nodiscard]] allocation_result<void> do_allocation(size_t size) noexcept;
	[[nodiscard]] void * do_reallocation(void * ptr, size_t new_size) noexcept;
	void do_deallocation(void * ptr) noexcept;

	struct allocator_arguments_marker {};
	inline constexpr allocator_arguments_marker allocator_arguments;
}

namespace gstd::string {
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
}

#endif // GSTD_ALLOCATION_HPP
