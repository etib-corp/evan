/*
** ETIB PROJECT, 2025
** maverik
** File description:
** ASwapchain
*/

#pragma once

#include <string>

namespace maverik {
template <typename T> struct ASwapchain {
  int32_t _width;
  int32_t _height;
  T swapchain;
};
} // namespace maverik