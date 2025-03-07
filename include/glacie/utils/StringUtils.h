#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "glacie/memory/Memory.h"

#include "fmt/color.h"
#include "fmt/core.h"

namespace glacie::utils::string_utils {

// "2021-03-24"  ->  ["2021", "03", "24"]  (use '-' as split pattern)
[[nodiscard]] constexpr std::vector<std::string_view>
splitByPattern(std::string_view s, std::string_view pattern, bool keepEmpty = false) {
    if (s.empty()) return {};
    size_t pos  = s.find(pattern);
    size_t size = s.size();

    std::vector<std::string_view> ret;
    while (pos != std::string::npos) {
        if (keepEmpty || pos != 0) ret.push_back(s.substr(0, pos));
        s   = s.substr(pos + pattern.size(), size - pos - pattern.size());
        pos = s.find(pattern);
    }
    if (keepEmpty || !s.empty()) ret.push_back(s);
    return ret;
}

/**
 * @brief Replace all founded sub std::string and modify input str
 * @param str       The input std::string
 * @param oldValue  The sub string to be replaced
 * @param newValue  The string to replace with
 * @return std::string  The modified input std::string
 */
constexpr std::string& replaceAll(std::string& str, std::string_view oldValue, std::string_view newValue) {
    for (std::string::size_type pos(0); pos != std::string::npos; pos += newValue.length()) {
        if ((pos = str.find(oldValue, pos)) != std::string::npos) str.replace(pos, oldValue.length(), newValue);
        else break;
    }
    return str;
}

[[nodiscard]] constexpr std::string
replaceAll(std::string const& str, std::string_view oldValue, std::string_view newValue) {
    std::string ret = str;
    replaceAll(ret, oldValue, newValue);
    return ret;
}

constexpr inline uint8_t digitFromByte[] = {
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 0,   1,   2,   3,   4,   5,   6,   7,   8,   9,   255, 255, 255, 255, 255, 255, 255, 10,
    11,  12,  13,  14,  15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,  32,
    33,  34,  35,  255, 255, 255, 255, 255, 255, 10,  11,  12,  13,  14,  15,  16,  17,  18,  19,  20,  21,  22,
    23,  24,  25,  26,  27,  28,  29,  30,  31,  32,  33,  34,  35,  255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255
};
static_assert(sizeof(digitFromByte) == 256);

[[nodiscard]] constexpr uint8_t digitFromChar(char chr) noexcept { return digitFromByte[static_cast<uint8_t>(chr)]; }

/**
 * @brief Integer to hex string.
 *
 * @tparam T      The integer type
 * @param  value  The integer value
 * @param  upper  Whether to use upper case (0x1A or 0x1a)
 * @param  no0x   Whether to omit 0x prefix
 * @param  noLeadingZero  Whether to omit leading zero
 * @return std::string    The hex string
 *
 * @par Example
 * @code
 * IntToHexStr(15); // "F"
 * IntToHexStr(16, true, true, false); // "0000000F"
 * @endcode
 */
template <class T>
    requires std::is_integral_v<T>
[[nodiscard]] constexpr std::string
intToHexStr(T value, bool upperCase = true, bool no0x = true, bool noLeadingZero = true) {
    std::string result;
    if (value < 0) result += '-';
    if (!no0x) result += "0x";
    constexpr char hexStr[2][17] = {"0123456789abcdef", "0123456789ABCDEF"};
    bool           leadingZero   = true;
    for (int i = sizeof(T) * 2; i > 0; --i) {
        auto hex = (value >> (i - 1) * 4) & 0xF;
        if (noLeadingZero && leadingZero && hex == 0) continue;
        leadingZero  = false;
        result      += hexStr[upperCase][hex];
    }
    return result;
}

[[nodiscard]] constexpr std::string strToHexStr(std::string_view value, bool upperCase = false, bool addSpace = false) {
    constexpr char hexStr[2][17] = {"0123456789abcdef", "0123456789ABCDEF"};
    std::string    hex;
    hex.reserve(value.size() * (addSpace ? 3 : 2));
    for (uint8_t x : value) {
        hex += hexStr[upperCase][x / 16];
        hex += hexStr[upperCase][x % 16];
        if (addSpace) hex += ' ';
    }
    if (addSpace && hex.ends_with(' ')) hex.pop_back();
    return hex;
}

[[nodiscard]] constexpr std::string applyTextStyle(fmt::text_style const& ts, std::string_view str) {
    std::string res;
    bool        has_style = false;
    if (ts.has_emphasis()) {
        has_style     = true;
        auto emphasis = fmt::detail::make_emphasis<char>(ts.get_emphasis());
        res.append(emphasis.begin(), emphasis.end());
    }
    if (ts.has_foreground()) {
        has_style       = true;
        auto foreground = fmt::detail::make_foreground_color<char>(ts.get_foreground());
        res.append(foreground.begin(), foreground.end());
    }
    if (ts.has_background()) {
        has_style       = true;
        auto background = fmt::detail::make_background_color<char>(ts.get_background());
        res.append(background.begin(), background.end());
    }
    res += str;
    if (has_style) res += "\x1b[0m";
    return res;
}

bool isu8str(std::string_view str) noexcept;

std::string tou8str(std::string_view str);

namespace CodePage {
enum : uint32_t {
    UTF16 = 0,
    ANSI  = 936,
    UTF8  = 65001,
};
} // namespace CodePage

std::wstring str2wstr(std::string_view str, uint32_t codePage = CodePage::UTF8);

std::string wstr2str(std::wstring_view str, uint32_t codePage = CodePage::UTF8);

std::string str2str(std::string_view str, uint32_t fromCodePage = CodePage::ANSI, uint32_t toCodePage = CodePage::UTF8);

[[nodiscard]] inline std::string u8str2str(std::u8string str) {
    std::string& tmp = *reinterpret_cast<std::string*>(&str);
    return {std::move(tmp)};
}

[[nodiscard]] inline std::u8string str2u8str(std::string str) {
    std::u8string& tmp = *reinterpret_cast<std::u8string*>(&str);
    return {std::move(tmp)};
}

[[nodiscard]] inline std::string const& u8str2strConst(std::u8string const& str) {
    return *reinterpret_cast<const std::string*>(&str);
}

[[nodiscard]] inline std::u8string const& str2u8strConst(std::string const& str) {
    return *reinterpret_cast<const std::u8string*>(&str);
}

[[nodiscard]] inline std::string_view u8sv2sv(std::u8string_view str) {
    return {reinterpret_cast<char const*>(str.data()), str.size()};
}

[[nodiscard]] inline std::u8string_view sv2u8sv(std::string_view str) {
    return {reinterpret_cast<const char8_t*>(str.data()), str.size()};
}
template <class T, auto f>
[[nodiscard]] inline T svtonum(std::string_view str, size_t* idx, int base) {
    int&        errnoRef = errno;
    char const* ptr      = str.data();
    char*       eptr;
    errnoRef       = 0;
    const auto ans = f(ptr, &eptr, base);
    if (ptr == eptr) { throw std::invalid_argument("invalid svtonum argument"); }
    if (errnoRef == ERANGE) { throw std::out_of_range("svtonum argument out of range"); }
    if (idx) { *idx = static_cast<size_t>(eptr - ptr); }
    return static_cast<T>(ans);
}
template <class T, auto f>
[[nodiscard]] inline T svtonum(std::string_view str, size_t* idx) {
    int&        errnoRef = errno;
    char const* ptr      = str.data();
    char*       eptr;
    errnoRef       = 0;
    const auto ans = f(ptr, &eptr);
    if (ptr == eptr) { throw std::invalid_argument("invalid svtonum argument"); }
    if (errnoRef == ERANGE) { throw std::out_of_range("svtonum argument out of range"); }
    if (idx) { *idx = static_cast<size_t>(eptr - ptr); }
    return static_cast<T>(ans);
}

[[nodiscard]] inline int8_t svtoc(std::string_view str, size_t* idx = nullptr, int base = 10) {
    return svtonum<int8_t, strtol>(str, idx, base);
}
[[nodiscard]] inline uint8_t svtouc(std::string_view str, size_t* idx = nullptr, int base = 10) {
    return svtonum<uint8_t, strtoul>(str, idx, base);
}
[[nodiscard]] inline short svtos(std::string_view str, size_t* idx = nullptr, int base = 10) {
    return svtonum<short, strtol>(str, idx, base);
}
[[nodiscard]] inline uint16_t svtous(std::string_view str, size_t* idx = nullptr, int base = 10) {
    return svtonum<uint16_t, strtoul>(str, idx, base);
}
[[nodiscard]] inline int svtoi(std::string_view str, size_t* idx = nullptr, int base = 10) {
    return svtonum<int, strtol>(str, idx, base);
}
[[nodiscard]] inline uint32_t svtoui(std::string_view str, size_t* idx = nullptr, int base = 10) {
    return svtonum<uint32_t, strtoul>(str, idx, base);
}
[[nodiscard]] inline long svtol(std::string_view str, size_t* idx = nullptr, int base = 10) {
    return svtonum<long, strtol>(str, idx, base);
}
[[nodiscard]] inline uint64_t svtoul(std::string_view str, size_t* idx = nullptr, int base = 10) {
    return svtonum<uint64_t, strtoul>(str, idx, base);
}
[[nodiscard]] inline int64_t svtoll(std::string_view str, size_t* idx = nullptr, int base = 10) {
    return svtonum<int64_t, strtoll>(str, idx, base);
}
[[nodiscard]] inline uint64_t svtoull(std::string_view str, size_t* idx = nullptr, int base = 10) {
    return svtonum<uint64_t, strtoull>(str, idx, base);
}
[[nodiscard]] inline float svtof(std::string_view str, size_t* idx = nullptr) {
    return svtonum<float, strtof>(str, idx);
}
[[nodiscard]] inline double svtod(std::string_view str, size_t* idx = nullptr) {
    return svtonum<double, strtof>(str, idx);
}
[[nodiscard]] inline long double svtold(std::string_view str, size_t* idx = nullptr) {
    return svtonum<long double, strtof>(str, idx);
}
} // namespace glacie::utils::string_utils