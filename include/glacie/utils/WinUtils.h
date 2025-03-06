#pragma once

#include <span>
#include <string>

namespace glacie::utils::win_utils {

std::string getSystemLocaleName();

bool isWine();

std::span<uint8_t> getImageRange(std::string const& name = "");

} // namespace glacie::utils::win_utils