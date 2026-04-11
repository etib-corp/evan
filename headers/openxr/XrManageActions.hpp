/*
** ETIB PROJECT, 2026
** evan
** File description:
** XrManageActions
*/

#pragma once

#include "openxr/actions/XrHandActions.hpp"

#include <memory>

namespace evan
{
    class XrDeviceBackend;
    /**
     * @brief The XrManageActions class is responsible for managing all action sets
     * and actions in an OpenXR application.
     *
     * This class serves as a central point for creating and managing action sets
     * and their associated actions, such as hand tracking actions. It provides
     * an interface for initializing action sets, defining actions, and handling
     * input from various XR devices. By encapsulating the action management
     * functionality, this class simplifies the process of integrating input
     * handling into an OpenXR application, allowing developers to focus on
     * creating immersive experiences.
     */
    class XrManageActions {
    public:
        XrManageActions(XrDeviceBackend &deviceBackend);

        ~XrManageActions();

        std::vector<std::shared_ptr<utility::event::Event>> pollActions(XrDeviceBackend &deviceBackend);

    private:
        XrActionSet _actionSet; // The main action set for the application

        std::unique_ptr<XrHandActions> _leftHandActions; // Actions for the left hand
        std::unique_ptr<XrHandActions> _rightHandActions; // Actions for the right hand
    };
} // namespace evan