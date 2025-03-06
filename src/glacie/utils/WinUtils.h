#pragma once

#include <span>
#include <string>

#include "glacie/base/StdInt.h"

namespace glacie::utils::win_utils {

std::string getSystemLocaleName();

bool isWine();

std::span<uchar> getImageRange(std::string const& name = "");

} // namespace glacie::utils::win_utils