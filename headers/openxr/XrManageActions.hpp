/*
** ETIB PROJECT, 2026
** evan
** File description:
** XrManageActions
*/

#pragma once

#include "openxr/actions/XrHandsMotionActions.hpp"
#include "openxr/actions/XrManageButtonsActions.hpp"

#include <memory>

namespace evan
{
    class XrDeviceBackend;

    /**
     * @class XrManageActions
     *
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
        /**
         * @brief Initializes XrManageActions with OpenXR action management.
         *
         * Constructs an XrManageActions instance and sets up all necessary OpenXR action
         * handling including action set creation, hands motion and button input actions,
         * space binding, and session attachment.
         *
         * @param deviceBackend Reference to the XrDeviceBackend instance that manages
         *                      the OpenXR device and session.
         *
         * @details The constructor performs the following initialization steps in order:
         *          1. Creates the main action set
         *          2. Initializes hands motion action handler
         *          3. Initializes buttons action handler
         *          4. Binds all action sets to the device backend
         *          5. Creates spaces for hands motion tracking
         *          6. Attaches the action set to the active session
         *
         * @note The order of operations is critical for proper OpenXR initialization.
         *
         * @see XrHandsMotionActions, XrManageButtonsActions
         */
        XrManageActions(XrDeviceBackend &deviceBackend);

        ~XrManageActions();

        std::vector<std::unique_ptr<utility::event::Event>> pollActions(XrDeviceBackend &deviceBackend);

        XrActionSet _actionSet; // The main action set for the application

        std::unique_ptr<XrHandsMotionActions> _handsMotionActions; // Action manager for hand tracking actions

        std::unique_ptr<XrManageButtonsActions> _manageButtonsActions; // Action manager for button actions
    private:
        void createActionSet(XrDeviceBackend &deviceBackend);

        void attachSessionActionSet(XrDeviceBackend &deviceBackend);

        void bindActionSets(XrDeviceBackend &deviceBackend);
    };
} // namespace evan