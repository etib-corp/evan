/*
** ETIB PROJECT, 2026
** evan
** File description:
** Software
*/

#pragma once

#include "DeviceContext.hpp"
#include "Version.hpp"

#include <string>
#include <memory>

namespace evan
{
    /**
     * @class Software
     * @brief Represents the software information of the engine, including its name and version.
     *
     * The Software class encapsulates details about the engine's software, such as its name and version. It provides a structured way to access this information throughout the engine.
     *
     * @var Software::_engineName
     *      The name of the engine.
     * @var Software::_engineVersion
     *      The version of the engine as a string.
     * @var Software::_version
     *      The version of the engine as a Version object.
     */
    class Software {
        public:
            Software();
            ~Software();

        protected:
            const std::string _engineName = "Evan Engine";                                  /// The name of the engine.
            const std::string _engineVersion = "0.1.0";                                     /// The version of the engine as a string.
            const std::shared_ptr<Version> _version = std::make_shared<Version>(0, 1, 0);   /// The version of the engine as a Version object.

            std::unique_ptr<DeviceContext> _deviceContext;    /// A unique pointer to the DeviceContext, which manages the device-related information and operations for the engine.
        private:
    };
} // namespace evan

