/*
** ETIB PROJECT, 2025
** evan
** File description:
** ASwapchain
*/

#pragma once

#include <string>

namespace evan {
template <typename T> struct ASwapchain {
  int32_t _width;
  int32_t _height;
  T swapchain;
};
} // namespace evan