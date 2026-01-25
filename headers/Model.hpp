/*
** ETIB PROJECT, 2026
** evan
** File description:
** Model
*/

#pragma once

#include "Primitive.hpp"
#include <string>

namespace evan {
    class Model : public Primitive {
        public:

            virtual ~Model() = default;

            virtual void loadFromFile(const std::string &filepath) = 0;

        protected:
            std::string _filepath;
        private:
    };
} // namespace evan
