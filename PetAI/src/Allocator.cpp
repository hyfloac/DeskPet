#include <cstdlib>
#include <SysLib.h>
#include <new>

void* operator new(const ::std::size_t size)
{
    return Alloc(size);
}

void* operator new[](const ::std::size_t size)
{
    return Alloc(size);
}

void* operator new(const ::std::size_t size, const ::std::nothrow_t&) noexcept
{
    return Alloc(size);
}

void* operator new[](const ::std::size_t size, const ::std::nothrow_t&) noexcept
{
    return Alloc(size);
}

void operator delete(void* const ptr) noexcept
{
    Free(ptr);
}

void operator delete[](void* const ptr) noexcept
{
    Free(ptr);
}

void operator delete(void* const ptr, const ::std::size_t) noexcept
{
    Free(ptr);
}

void operator delete[](void* const ptr, const ::std::size_t) noexcept
{
    Free(ptr);
}

void operator delete(void* const ptr, const ::std::nothrow_t&) noexcept
{
    Free(ptr);
}

void operator delete[](void* const ptr, const ::std::nothrow_t&) noexcept
{
    Free(ptr);
}
