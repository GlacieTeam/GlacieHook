#include "glacie/utils/WinUtils.h"

#include <string>

#include "glacie/utils/StringUtils.h"

#include "windows.h"

#include "psapi.h"

using namespace glacie::utils::string_utils;
namespace glacie::utils::win_utils {

std::string getSystemLocaleName() {
    wchar_t buf[LOCALE_NAME_MAX_LENGTH]{};
    GetSystemDefaultLocaleName(buf, LOCALE_NAME_MAX_LENGTH);
    auto str = wstr2str(buf);
    replaceAll(str, "-", "_");
    return str;
}

bool isWine() {
    static bool result = []() {
        HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");
        if (!ntdll) return false;
        auto funcWineGetVersion = GetProcAddress(ntdll, "wine_get_version");
        if (funcWineGetVersion) return true;
        else return false;
    }();
    return result;
}

std::span<uint8_t> getImageRange(std::string const& name) {
    static auto process = GetCurrentProcess();
    HMODULE     rangeStart;
    if (name.empty()) {
        rangeStart = GetModuleHandle(nullptr);
    } else {
        rangeStart = GetModuleHandle(str2wstr(name).c_str());
    }
    if (rangeStart) {
        MODULEINFO miModInfo;
        if (GetModuleInformation(process, rangeStart, &miModInfo, sizeof(MODULEINFO))) {
            return {(uint8_t*)rangeStart, miModInfo.SizeOfImage};
        }
    }
    return {};
}

} // namespace glacie::utils::win_utils