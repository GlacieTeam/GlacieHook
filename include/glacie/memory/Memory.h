#pragma once

#include <cstddef>
#include <cstring>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

#include "glacie/base/FixedString.h"
#include "glacie/utils/StringUtils.h"
#include "libhat/Signature.hpp"

namespace glacie::memory {

using FuncPtr = void*;
using Handle  = void*;

extern "C" struct _IMAGE_DOS_HEADER __ImageBase; // NOLINT(bugprone-reserved-identifier)

template <class T>
    requires(sizeof(T) == sizeof(FuncPtr))
constexpr FuncPtr toFuncPtr(T t) {
    union {
        FuncPtr fp;
        T       t;
    } u{};
    u.t = t;
    return u.fp;
}

template <class T>
    requires(std::is_member_function_pointer_v<T> && sizeof(T) == sizeof(FuncPtr) + sizeof(ptrdiff_t))
constexpr FuncPtr toFuncPtr(T t) {
    union {
        struct {
            FuncPtr   fp;
            ptrdiff_t offset;
        };
        T t;
    } u{};
    u.t = t;
    return u.fp;
}

template <class T>
inline void memcpy_t(void* dst, void const* src, size_t count) {
    memcpy(dst, src, count * sizeof(T));
}

template <class T>
inline void memcpy_t(void* dst, void const* src) {
    memcpy(dst, src, sizeof(T));
}

/**
 * @brief resolve signature to function pointer
 * @param t Signature
 * @return function pointer
 */
FuncPtr resolveSignature(const char* signature);

/**
 * @brief make a region of memory writable and executable, then call the
 * callback, and finally restore the region.
 * @param ptr Pointer to the region
 * @param len Length of the region
 * @param callback Callback
 */
void modify(void* ptr, size_t len, const std::function<void()>& callback);

Handle getModuleHandle(void* addr);

inline Handle getCurrentModuleHandle() { return &__ImageBase; }

template <class T>
inline void modify(T& ref, std::function<void(std::remove_cvref_t<T>&)> const& f) {
    modify((void*)std::addressof(ref), sizeof(T), [&] { f((std::remove_cvref_t<T>&)(ref)); });
}

template <class R = void, class... Args>
constexpr auto addressCall(void const* address, auto&&... args) -> R {
    return ((R(*)(Args...))address)(std::forward<decltype((args))>(args)...);
}

template <class R = void, class... Args>
constexpr auto virtualCall(void const* self, ptrdiff_t vIndex, auto&&... args) -> R {
    return (*(R(**)(void const*, Args...))(*(uintptr_t**)self + vIndex))(self, std::forward<decltype((args))>(args)...);
}

template <class T>
[[nodiscard]] constexpr T& dAccess(void* ptr, ptrdiff_t off) {
    return *(T*)((uintptr_t)((uintptr_t)ptr + off));
}

template <class T>
[[nodiscard]] constexpr T const& dAccess(void const* ptr, ptrdiff_t off) {
    return *(T*)((uintptr_t)((uintptr_t)ptr + off));
}

template <class T>
constexpr void destruct(void* ptr, ptrdiff_t off) noexcept {
    std::destroy_at(std::launder(reinterpret_cast<T*>((uintptr_t)((uintptr_t)ptr + off))));
}

template <class T, class... Args>
constexpr auto construct(void* ptr, ptrdiff_t off, Args&&... args) {
    return std::construct_at(
        std::launder(reinterpret_cast<T*>((uintptr_t)((uintptr_t)ptr + off))),
        std::forward<Args>(args)...
    );
}

[[nodiscard]] inline size_t getMemSizeFromPtr(void* ptr) {
    if (!ptr) { return 0; }
    return _msize(ptr);
}

template <class T, class D>
[[nodiscard]] inline size_t getMemSizeFromPtr(std::unique_ptr<T, D>& ptr) {
    if (!ptr) { return 0; }
    return _msize(ptr.get());
}

template <template <class> class P, class T>
[[nodiscard]] inline size_t getMemSizeFromPtr(P<T>& ptr)
    requires(std::derived_from<P<T>, std::_Ptr_base<T>>)
{
    auto& refc = dAccess<std::_Ref_count_base*>(std::addressof(ptr), 8);
    if (!refc) { return 0; }
    auto& rawptr = dAccess<T*>(std::addressof(ptr), 0);
    if (!rawptr) { return 0; }
    if constexpr (!std::is_array_v<T>) {
        if (rawptr == dAccess<T*>(refc, 8 + 4 * 2)) { return getMemSizeFromPtr(rawptr); }
    }
    // clang-format off
    return _msize(refc // ptr* 8, rep* 8
    ) - ( // rep:
    8 +                        // vtable
    4 * 2 +                    // uses & weaks
   (std::is_array_v<T> ? 8 : 0)// size
    /**/                       // storage
    );
    // clang-format on
}

template <FixedString signature>
inline FuncPtr signatureCache = resolveSignature(signature);

} // namespace glacie::memory

#define RESOLVE_SIGNATURE(signature) (glacie::memory::signatureCache<signature>)

#define ADDRESS_CALL(address, Ret, ...) ((Ret(*)(__VA_ARGS__))(address))

#define SIGNATURE_CALL(signature, Ret, ...) ((Ret(*)(__VA_ARGS__))(glacie::memory::signatureCache<signature>))