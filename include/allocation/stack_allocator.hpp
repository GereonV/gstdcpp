#ifndef GSTD_ALLOCATION_STACK_ALLOCATION_HPP
#define GSTD_ALLOCATION_STACK_ALLOCATION_HPP

#include "allocation/base.hpp"
#include "allocation/buffer_allocator.hpp"

namespace gstd::allocation {
    namespace _impl {
        template<size_t N, size_t StartAlignment = 1>
        class stack_allocator_base {
          public:
            constexpr stack_allocator_base() noexcept {}

            stack_allocator_base(stack_allocator_base const &) = delete;
            void operator=(stack_allocator_base const &)       = delete;
          protected:
            constexpr allocation_result * get_allocation() const noexcept { return {_data, N}; }
          private:
            alignas(StartAlignment) char _data[N];
        };
    }

    template<size_t N, size_t StartAlignment = 1>
    struct stack_allocator : _impl::stack_allocator_base<N, StartAlignment>,
                             _impl::buffer_allocator_base<stack_allocator<N, StartAlignment>> {
        friend _impl::buffer_allocator_base<stack_allocator<N, StartAlignment>>;
        using _impl::stack_allocator_base<N, StartAlignment>::stack_allocator_base;
    };
}

#endif
