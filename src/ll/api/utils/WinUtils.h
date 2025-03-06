#pragma once

#include <span>
#include <string>

#include "ll/api/base/StdInt.h"

namespace ll::utils::win_utils {

std::string getSystemLocaleName();

bool isWine();

std::span<uchar> getImageRange(std::string const& name = "");

} // namespace ll::utils::win_utils