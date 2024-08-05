#ifndef GSTD_ALLOCATION_CREATE_DESTROY_HPP
#define GSTD_ALLOCATION_CREATE_DESTROY_HPP

#include <concepts>
#include <type_traits>
#include "allocation/base.hpp"

namespace gstd::allocation {
    template<typename T, typename... Args>
    requires (std::constructible_from<T, Args...>)
    constexpr creation_result<T>
    create(allocator auto & alloc, Args &&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>)
    {
        if consteval {
            return {new T(static_cast<Args &&>(args)...), sizeof(T)};
        } else {
            auto result = alloc.allocate(sizeof(T));
            if(!result)
                return {nullptr, 0};
            return {new (result.ptr) T(static_cast<Args &&>(args)...), result.size};
        }
    }

    template<typename T>
    constexpr void destroy(allocator auto & alloc, creation_result<T> creation) noexcept
    {
        if consteval {
            delete creation.ptr;
        } else {
            creation.ptr->~T();
            alloc.deallocate(allocation_result{creation.ptr, creation.size});
        }
    }
}

#endif
