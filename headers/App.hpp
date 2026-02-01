/*
** ETIB PROJECT, 2026
** evan
** File description:
** App
*/

#pragma once

#include "ASoftware.hpp"
#include "Primitive.hpp"

namespace evan {
    class App {
        public:

            struct AppProperties {
                std::string appName;
                Version appVersion;
                const std::string engineName = "Evan";
                const Version engineVersion = Version(0, 2, 0);
            };

            App(const AppProperties &properties);
            ~App();

            virtual std::shared_ptr<Primitive> &display(std::shared_ptr<Primitive> primitive);

            virtual void drawFrame(float frameRate);

            void run();

            // virtual bool shouldClose() = 0;

        protected:
        private:
            std::unique_ptr<ASoftware> _software;
            std::vector<std::shared_ptr<Primitive>> _primitives;
    };
} // namespace evan

