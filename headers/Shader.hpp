/*
** ETIB PROJECT, 2025
** evan
** File description:
** Shader
*/

#pragma once

#include "Evan.hpp"

/**
 * @namespace evan
 * @brief The evan namespace contains classes and functions for the evan
 * project.
 */
namespace evan {
    /**
     * @class Shader
     * @brief The Shader class represents a shader used in rendering.
     */
    class Shader {
        public:
            Shader(const std::vector<uint32_t> &vertexCode,
                const std::vector<uint32_t> &fragmentCode,
                VkDevice device);
            ~Shader();

        protected:
            VkShaderModule _vertexShaderModule;
            VkShaderModule _fragmentShaderModule;
        private:
    };
} // namespace evan
