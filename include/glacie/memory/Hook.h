#pragma once

#include <memory>
#include <set>
#include <type_traits>

#include "glacie/memory/Memory.h"

namespace glacie::memory {

typedef void* FuncPtr;

int hook(FuncPtr target, FuncPtr detour, FuncPtr* originalFunc);

bool unhook(FuncPtr target, FuncPtr detour);

template <class T>
struct IsConstMemberFun : std::false_type {};

template <class T, class Ret, class... Args>
struct IsConstMemberFun<Ret (T::*)(Args...) const> : std::true_type {};

template <class T>
inline constexpr bool IsConstMemberFunV = IsConstMemberFun<T>::value;

template <class T>
struct AddConstAtMemberFun {
    using type = T;
};

template <class T, class Ret, class... Args>
struct AddConstAtMemberFun<Ret (T::*)(Args...)> {
    using type = Ret (T::*)(Args...) const;
};

template <class T>
using AddConstAtMemberFunT = typename AddConstAtMemberFun<T>::type;

template <class T, class U>
using AddConstAtMemberFunIfOriginIs = std::conditional_t<IsConstMemberFunV<U>, AddConstAtMemberFunT<T>, T>;

/**
 * @brief Get the pointer of a function by identifier.
 *
 * @param identifier signature
 * @return FuncPtr
 */
FuncPtr resolveIdentifier(char const* identifier);

template <class T>
concept FuncPtrType = std::is_function_v<std::remove_pointer_t<T>> || std::is_member_function_pointer_v<T>;

template <class T>
    requires(FuncPtrType<T> || std::is_same_v<T, uintptr_t>)
constexpr FuncPtr resolveIdentifier(T identifier) {
    return toFuncPtr(identifier);
}

// redirect to resolveIdentifier(char const*)
template <class T>
constexpr FuncPtr resolveIdentifier(char const* identifier) {
    return resolveIdentifier(identifier);
}

// redirect to resolveIdentifier(uintptr_t)
template <class T>
constexpr FuncPtr resolveIdentifier(uintptr_t address) {
    return resolveIdentifier(address);
}

template <class... Ts>
class HookRegistrar {
public:
    static void hook() { (((++Ts::AutoHookCount == 1) ? Ts::hook() : 0), ...); }
    static void unhook() { (((--Ts::AutoHookCount == 0) ? Ts::unhook() : 0), ...); }
    HookRegistrar() noexcept { hook(); }
    ~HookRegistrar() noexcept { unhook(); }
    HookRegistrar(HookRegistrar const&) noexcept { ((++Ts::AutoHookCount), ...); }
    HookRegistrar& operator=(HookRegistrar const& other) noexcept {
        if (this != std::addressof(other)) { ((++Ts::AutoHookCount), ...); }
        return *this;
    }
    HookRegistrar(HookRegistrar&&) noexcept            = default;
    HookRegistrar& operator=(HookRegistrar&&) noexcept = default;
};

struct Hook {};

} // namespace glacie::memory

#define VA_EXPAND(...) __VA_ARGS__

#define GLACIE_HOOK_IMPL(REGISTER, FUNC_PTR, STATIC, CALL, DEF_TYPE, TYPE, IDENTIFIER, RET_TYPE, ...)                  \
    struct DEF_TYPE : public TYPE {                                                                                    \
        inline static ::std::atomic_uint AutoHookCount{};                                                              \
                                                                                                                       \
    private:                                                                                                           \
        using FuncPtr = ::glacie::memory::FuncPtr;                                                                     \
        using OriginFuncType =                                                                                         \
            ::glacie::memory::AddConstAtMemberFunIfOriginIs<RET_TYPE FUNC_PTR(__VA_ARGS__), decltype(IDENTIFIER)>;     \
                                                                                                                       \
        inline static FuncPtr        HookTarget{};                                                                     \
        inline static OriginFuncType OriginalFunc{};                                                                   \
                                                                                                                       \
    public:                                                                                                            \
        template <class... Args>                                                                                       \
        STATIC RET_TYPE origin(Args&&... params) {                                                                     \
            return CALL(std::forward<Args>(params)...);                                                                \
        }                                                                                                              \
                                                                                                                       \
        STATIC RET_TYPE detour(__VA_ARGS__);                                                                           \
                                                                                                                       \
        static int hook() {                                                                                            \
            HookTarget = glacie::memory::resolveIdentifier<OriginFuncType>(IDENTIFIER);                                \
            if (HookTarget == nullptr) { return -1; }                                                                  \
            return glacie::memory::hook(                                                                               \
                HookTarget,                                                                                            \
                glacie::memory::toFuncPtr(&DEF_TYPE::detour),                                                          \
                reinterpret_cast<FuncPtr*>(&OriginalFunc)                                                              \
            );                                                                                                         \
        }                                                                                                              \
                                                                                                                       \
        static bool unhook() {                                                                                         \
            return glacie::memory::unhook(HookTarget, glacie::memory::toFuncPtr(&DEF_TYPE::detour));                   \
        }                                                                                                              \
    };                                                                                                                 \
    REGISTER;                                                                                                          \
    RET_TYPE DEF_TYPE::detour(__VA_ARGS__)

#define GLACIE_AUTO_REG_HOOK_IMPL(FUNC_PTR, STATIC, CALL, DEF_TYPE, ...)                                               \
    VA_EXPAND(GLACIE_HOOK_IMPL(                                                                                        \
        inline ::glacie::memory::HookRegistrar<DEF_TYPE> DEF_TYPE##AutoRegister,                                       \
        FUNC_PTR,                                                                                                      \
        STATIC,                                                                                                        \
        CALL,                                                                                                          \
        DEF_TYPE,                                                                                                      \
        __VA_ARGS__                                                                                                    \
    ))

#define GLACIE_MANUAL_REG_HOOK_IMPL(...) VA_EXPAND(GLACIE_HOOK_IMPL(, __VA_ARGS__))

#define GLACIE_STATIC_HOOK_IMPL(...) VA_EXPAND(GLACIE_MANUAL_REG_HOOK_IMPL((*), static, OriginalFunc, __VA_ARGS__))

#define GLACIE_AUTO_STATIC_HOOK_IMPL(...) VA_EXPAND(GLACIE_AUTO_REG_HOOK_IMPL((*), static, OriginalFunc, __VA_ARGS__))

#define GLACIE_INSTANCE_HOOK_IMPL(DEF_TYPE, ...)                                                                       \
    VA_EXPAND(GLACIE_MANUAL_REG_HOOK_IMPL((DEF_TYPE::*), , (this->*OriginalFunc), DEF_TYPE, __VA_ARGS__))

#define GLACIE_AUTO_INSTANCE_HOOK_IMPL(DEF_TYPE, ...)                                                                  \
    VA_EXPAND(GLACIE_AUTO_REG_HOOK_IMPL((DEF_TYPE::*), , (this->*OriginalFunc), DEF_TYPE, __VA_ARGS__))

/**
 * @brief Register a hook for a typed static function.
 * @param DEF_TYPE The name of the hook definition.
 * @param TYPE The type which the function belongs to.
 * @param IDENTIFIER The identifier of the hook. It can be a function pointer, symbol, address or a signature.
 * @param RET_TYPE The return type of the hook.
 * @param ... The parameters of the hook.
 *
 * @note register or unregister by calling DEF_TYPE::hook() and DEF_TYPE::unhook().
 */
#define GLACIE_TYPE_STATIC_HOOK(DEF_TYPE, TYPE, IDENTIFIER, RET_TYPE, ...)                                             \
    VA_EXPAND(GLACIE_STATIC_HOOK_IMPL(DEF_TYPE, TYPE, IDENTIFIER, RET_TYPE, __VA_ARGS__))

/**
 * @brief Register a hook for a static function.
 * @param DEF_TYPE The name of the hook definition.
 * @param IDENTIFIER The identifier of the hook. It can be a function pointer, symbol, address or a signature.
 * @param RET_TYPE The return type of the hook.
 * @param ... The parameters of the hook.
 *
 * @note register or unregister by calling DEF_TYPE::hook() and DEF_TYPE::unhook().
 */
#define GLACIE_STATIC_HOOK(DEF_TYPE, IDENTIFIER, RET_TYPE, ...)                                                        \
    VA_EXPAND(GLACIE_STATIC_HOOK_IMPL(DEF_TYPE, ::glacie::memory::Hook, IDENTIFIER, RET_TYPE, __VA_ARGS__))

/**
 * @brief Register a hook for a typed static function.
 * @details The hook will be automatically registered and unregistered.
 * @see TYPE_STATIC_HOOK for usage.
 */
#define GLACIE_AUTO_TYPE_STATIC_HOOK(DEF_TYPE, TYPE, IDENTIFIER, RET_TYPE, ...)                                        \
    VA_EXPAND(GLACIE_AUTO_STATIC_HOOK_IMPL(DEF_TYPE, TYPE, IDENTIFIER, RET_TYPE, __VA_ARGS__))

/**
 * @brief Register a hook for a static function.
 * @details The hook will be automatically registered and unregistered.
 * @see STATIC_HOOK for usage.
 */
#define GLACIE_AUTO_STATIC_HOOK(DEF_TYPE, IDENTIFIER, RET_TYPE, ...)                                                   \
    VA_EXPAND(GLACIE_AUTO_STATIC_HOOK_IMPL(DEF_TYPE, ::glacie::memory::Hook, IDENTIFIER, RET_TYPE, __VA_ARGS__))

/**
 * @brief Register a hook for a typed instance function.
 * @param DEF_TYPE The name of the hook definition.
 * @param TYPE The type which the function belongs to.
 * @param IDENTIFIER The identifier of the hook. It can be a function pointer, symbol, address or a signature.
 * @param RET_TYPE The return type of the hook.
 * @param ... The parameters of the hook.
 *
 * @note register or unregister by calling DEF_TYPE::hook() and DEF_TYPE::unhook().
 */
#define GLACIE_TYPE_INSTANCE_HOOK(DEF_TYPE, TYPE, IDENTIFIER, RET_TYPE, ...)                                           \
    VA_EXPAND(GLACIE_INSTANCE_HOOK_IMPL(DEF_TYPE, TYPE, IDENTIFIER, RET_TYPE, __VA_ARGS__))

/**
 * @brief Register a hook for a instance function.
 * @param DEF_TYPE The name of the hook definition.
 * @param IDENTIFIER The identifier of the hook. It can be a function pointer, symbol, address or a signature.
 * @param RET_TYPE The return type of the hook.
 * @param ... The parameters of the hook.
 *
 * @note register or unregister by calling DEF_TYPE::hook() and DEF_TYPE::unhook().
 */
#define GLACIE_INSTANCE_HOOK(DEF_TYPE, IDENTIFIER, RET_TYPE, ...)                                                      \
    VA_EXPAND(GLACIE_INSTANCE_HOOK_IMPL(DEF_TYPE, ::glacie::memory::Hook, IDENTIFIER, RET_TYPE, __VA_ARGS__))

/**
 * @brief Register a hook for a typed instance function.
 * @details The hook will be automatically registered and unregistered.
 * @see TYPE_INSTANCE_HOOK for usage.
 */
#define GLACIE_AUTO_TYPE_INSTANCE_HOOK(DEF_TYPE, TYPE, IDENTIFIER, RET_TYPE, ...)                                      \
    VA_EXPAND(GLACIE_AUTO_INSTANCE_HOOK_IMPL(DEF_TYPE, TYPE, IDENTIFIER, RET_TYPE, __VA_ARGS__))

/**
 * @brief Register a hook for a instance function.
 * @details The hook will be automatically registered and unregistered.
 * @see INSTANCE_HOOK for usage.
 */
#define GLACIE_AUTO_INSTANCE_HOOK(DEF_TYPE, IDENTIFIER, RET_TYPE, ...)                                                 \
    VA_EXPAND(GLACIE_AUTO_INSTANCE_HOOK_IMPL(DEF_TYPE, ::glacie::memory::Hook, IDENTIFIER, RET_TYPE, __VA_ARGS__))