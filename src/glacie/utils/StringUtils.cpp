#include "glacie/utils/StringUtils.h"

#include "magic_enum.hpp"

#include "stringapiset.h"
#include <sstream>

namespace glacie::utils::string_utils {

fmt::text_style getTextStyleFromCode(std::string_view code) {
    if (code.starts_with("§")) {
        switch (code[2]) {
            // clang-format off
        default :
        case 'f': return fmt::fg(fmt::terminal_color::bright_white);
        case '0': return fmt::fg(fmt::terminal_color::black);
        case '1': return fmt::fg(fmt::rgb(0x0000AA));
        case '2': return fmt::fg(fmt::rgb(0x00AA00));
        case '3': return fmt::fg(fmt::rgb(0x00AAAA));
        case '4': return fmt::fg(fmt::rgb(0xAA0000));
        case '5': return fmt::fg(fmt::rgb(0xAA00AA));
        case '6': return fmt::fg(fmt::rgb(0xFFAA00));
        case '7': return fmt::fg(fmt::rgb(0xAAAAAA));
        case '8': return fmt::fg(fmt::rgb(0x555555));
        case '9': return fmt::fg(fmt::rgb(0x5555FF));
        case 'a': return fmt::fg(fmt::rgb(0x55FF55));
        case 'b': return fmt::fg(fmt::rgb(0x55FFFF));
        case 'c': return fmt::fg(fmt::rgb(0xFF5555));
        case 'd': return fmt::fg(fmt::rgb(0xFF55FF));
        case 'e': return fmt::fg(fmt::rgb(0xFFFF55));
        case 'g': return fmt::fg(fmt::rgb(0xDDD605));
        case 'h': return fmt::fg(fmt::rgb(0xE3D4D1));
        case 'i': return fmt::fg(fmt::rgb(0xCECACA));
        case 'j': return fmt::fg(fmt::rgb(0x443A3B));
        case 'm': return fmt::fg(fmt::rgb(0x971607));
        case 'n': return fmt::fg(fmt::rgb(0xB4684D));
        case 'p': return fmt::fg(fmt::rgb(0xDEB12D));
        case 'q': return fmt::fg(fmt::rgb(0x47A036));
        case 's': return fmt::fg(fmt::rgb(0x2CBAA8));
        case 't': return fmt::fg(fmt::rgb(0x21497B));
        case 'u': return fmt::fg(fmt::rgb(0x9A5CC6));
        case 'k': return fmt::emphasis::blink;
        case 'l': return fmt::emphasis::bold;
        case 'o': return fmt::emphasis::italic;
        case 'r': return {};
            // clang-format on
        }
    } else if (code.starts_with('\x1B')) {
        code.remove_prefix(2);
        code.remove_suffix(1);
        if (code.find(';') != std::string_view::npos) {
            bool background = code.starts_with('4');
            code.remove_prefix(1);
            if (!code.starts_with("8;2;")) { return {}; }
            code.remove_prefix(4);
            auto svec = splitByPattern(code, ";");
            if (svec.size() != 3) { return {}; }
            auto colorFromCode = fmt::rgb(svtouc(svec[0]), svtouc(svec[1]), svtouc(svec[2]));
            if (background) {
                return fmt::bg(colorFromCode);
            } else {
                return fmt::fg(colorFromCode);
            }
        } else {
            int num = svtoi(code);
            if (magic_enum::enum_contains<fmt::terminal_color>((uint8_t)num)) {
                return fmt::fg((fmt::terminal_color)num);
            } else if (magic_enum::enum_contains<fmt::terminal_color>((uint8_t)(num - 10))) {
                return fmt::bg((fmt::terminal_color)(num - 10));
            } else {
                switch (num) {
                    // clang-format off
                    case 1: return fmt::emphasis::bold;
                    case 2: return fmt::emphasis::faint;
                    case 3: return fmt::emphasis::italic;
                    case 4: return fmt::emphasis::underline;
                    case 5: return fmt::emphasis::blink;
                    case 7: return fmt::emphasis::reverse;
                    case 8: return fmt::emphasis::conceal;
                    case 9: return fmt::emphasis::strikethrough;
                    default: break;
                    // clang-format on
                }
            }
        }
    }

    return {};
}

std::string getAnsiCodeFromTextStyle(fmt::text_style style) {
    fmt::basic_memory_buffer<char> buf;
    bool                           has_style = false;
    if (style.has_emphasis()) {
        has_style     = true;
        auto emphasis = fmt::detail::make_emphasis<char>(style.get_emphasis());
        buf.append(emphasis.begin(), emphasis.end());
    }
    if (style.has_foreground()) {
        has_style       = true;
        auto foreground = fmt::detail::make_foreground_color<char>(style.get_foreground());
        buf.append(foreground.begin(), foreground.end());
    }
    if (!has_style) fmt::detail::reset_color<char>(buf);
    return fmt::to_string(buf);
}

std::wstring str2wstr(std::string_view str, uint32_t codePage) {
    int len = MultiByteToWideChar(codePage, 0, str.data(), (int)str.size(), nullptr, 0);
    if (len == 0) { return {}; }
    std::wstring wstr(len, L'\0');
    MultiByteToWideChar(codePage, 0, str.data(), (int)str.size(), wstr.data(), len);
    return wstr;
}

std::string wstr2str(std::wstring_view str, uint32_t codePage) {
    int len = WideCharToMultiByte(codePage, 0, str.data(), (int)str.size(), nullptr, 0, nullptr, nullptr);
    if (len == 0) { return {}; }
    std::string ret(len, '\0');
    WideCharToMultiByte(codePage, 0, str.data(), (int)str.size(), ret.data(), (int)ret.size(), nullptr, nullptr);
    return ret;
}

std::string str2str(std::string_view str, uint32_t fromCodePage, uint32_t toCodePage) {
    return wstr2str(str2wstr(str, fromCodePage), toCodePage);
}

bool isu8str(std::string_view str) noexcept {
    bool res = true;
    fmt::detail::for_each_codepoint(str, [&](uint32_t cp, fmt::string_view) {
        if (cp == fmt::detail::invalid_code_point) {
            res = false;
            return false;
        }
        return true;
    });
    return res;
}

std::string tou8str(std::string_view str) {
    if (isu8str(str)) {
        return std::string{str};
    } else {
        auto res = str2str(str);
        return isu8str(res) ? res : "unknown codepage";
    }
}

} // namespace glacie::utils::string_utils