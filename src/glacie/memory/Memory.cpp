#include "glacie/memory/Memory.h"

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

#include "glacie/base/StdInt.h"
#include "glacie/utils/StringUtils.h"
#include "glacie/utils/WinUtils.h"
#include "libhat/Process.hpp"
#include "libhat/Scanner.hpp"
#include "libhat/Signature.hpp"

#include "windows.h"
#include <libhat.hpp>
#include <winnt.h>

using namespace glacie::utils;

namespace glacie::memory {

FuncPtr resolveSignature(const char* signature) {
    auto module = hat::process::get_module("bedrock_server.exe");
    if (!module.has_value()) return nullptr;
    auto                                moduleData = hat::process::get_module_data(module.value());
    std::vector<hat::signature_element> elements;
    for (std::string_view const& sv : glacie::utils::string_utils::splitByPattern(signature, " ")) {
        if (sv.starts_with('?')) {
            elements.emplace_back();
        } else {
            elements.emplace_back((std::byte)(
                glacie::utils::string_utils::digitFromChar(sv[0]) << 4
                | glacie::utils::string_utils::digitFromChar(sv[1])
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

} // namespace glacie::memory