#include "ll/api/memory/Memory.h"

#include <cstddef>
#include <functional>
#include <iostream>
#include <libloaderapi.h>
#include <memoryapi.h>
#include <minwindef.h>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

#include "libhat/Process.hpp"
#include "libhat/Scanner.hpp"
#include "libhat/Signature.hpp"
#include "ll/api/base/StdInt.h"
#include "ll/api/utils/StringUtils.h"
#include "ll/api/utils/WinUtils.h"

#include <libhat.hpp>
#include <winnt.h>
#include "windows.h"

using namespace ll::utils;

namespace ll::memory {

FuncPtr resolveSignature(const char* signature) {
    auto module = hat::process::get_module("bedrock_server.exe");
    if (!module.has_value()) return nullptr;
    auto                                moduleData = hat::process::get_module_data(module.value());
    std::vector<hat::signature_element> elements;
    for (std::string_view const& sv : ll::utils::string_utils::splitByPattern(signature, " ")) {
        if (sv.starts_with('?')) {
            elements.emplace_back();
        } else {
            elements.emplace_back((std::byte)(
                ll::utils::string_utils::digitFromChar(sv[0]) << 4 | ll::utils::string_utils::digitFromChar(sv[1])
            ));
        }
    }
    auto result = hat::find_pattern(moduleData.begin(), moduleData.end(), hat::signature_view(elements));
    return reinterpret_cast<FuncPtr>(result.get());
}

void modify(void* ptr, size_t len, const std::function<void()>& callback) {
    DWORD oldProtect;
    VirtualProtect(ptr, len, PAGE_EXECUTE_READWRITE, &oldProtect);
    callback();
    VirtualProtect(ptr, len, oldProtect, &oldProtect);
}

Handle getModuleHandle(void* addr) {
    HMODULE hModule = nullptr;
    GetModuleHandleEx(
        GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        reinterpret_cast<LPCTSTR>(addr),
        &hModule
    );
    return hModule;
}

} // namespace ll::memory