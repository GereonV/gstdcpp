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

#endif // GSTD_ALLOCATION_HPP
