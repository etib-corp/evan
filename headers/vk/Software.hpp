/*
** ETIB PROJECT, 2025
** maverik
** File description:
** Software
*/

#pragma once

#include "ASoftware.hpp"
#include "vk/GraphicalContext.hpp"

#include <algorithm>
#include <cstring>
#include <iostream>

namespace maverik {
namespace vk {
class Software : public ASoftware {
public:
  // Constructors
  Software();
  Software(const std::string &appName, const Version &appVersion,
           const std::string &engineName, const Version &engineVersion);

  // Destructor
  ~Software();
};
} // namespace vk
} // namespace maverik
