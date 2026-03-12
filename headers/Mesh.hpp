/*
** ETIB PROJECT, 2026
** evan
** File description:
** Mesh
*/

#pragma once

#include "EvanPlatform.hpp"

#include "Vertex.hpp"

namespace evan {
    struct Mesh {
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        std::string materialPath;
    };
}
