/*
** ETIB PROJECT, 2026
** evan
** File description:
** XrManageActions
*/

#pragma once

#include "openxr/actions/XrHandsMotionActions.hpp"
#include "openxr/actions/XrManageButtonsActions.hpp"
#include "openxr/actions/XrManageThumbStickActions.hpp"

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

        /**
         * @brief Polls for input events from the OpenXR runtime.
         *
         * This method queries the OpenXR runtime for the current state of all managed actions, including hand motion, button input, and thumb stick input. It generates a vector of events based on the retrieved state, which can be used by the application to respond to user input. The events may include information about hand positions, button presses, and thumb stick movements detected by the OpenXR runtime.
         *
         * @param deviceBackend Reference to the XrDeviceBackend instance that manages the OpenXR device and session.
         * @return A vector of unique pointers to events representing the current state of all managed actions.
         */
        std::vector<std::unique_ptr<utility::event::Event>> pollActions(XrDeviceBackend &deviceBackend);

        XrActionSet _actionSet; // The main action set for the application

        std::unique_ptr<XrHandsMotionActions> _handsMotionActions; // Action manager for hand tracking actions

        std::unique_ptr<XrManageButtonsActions> _manageButtonsActions; // Action manager for button actions

        std::unique_ptr<XrManageThumbStickActions> _manageThumbStickActions; // Action manager for thumb stick actions
    private:
        /**
         * @brief Creates the main action set for the application.
         *
         * This method initializes the main action set that will be used to group all related actions for the application.
         * It sets up the necessary OpenXR structures and registers the action set with the OpenXR runtime.
         * The created action set will serve as the parent for all individual actions related to hand tracking, button input, and thumb stick input.
         *
         * @param deviceBackend Reference to the XrDeviceBackend instance that manages the OpenXR device and session, used for registering the action set with the runtime.
         */
        void createActionSet(XrDeviceBackend &deviceBackend);

        /**
         * @brief Creates spaces for hand motion tracking.
         *
         * This method sets up the necessary action spaces for hand motion tracking, allowing the application to track the position and orientation of the user's hands in the XR environment. It creates action spaces for both hand aim and hand grip actions, which are essential for accurate hand tracking and interaction within the XR experience.
         *
         * @param deviceBackend Reference to the XrDeviceBackend instance that manages the OpenXR device and session, used for creating action spaces with the runtime.
         */
        void attachSessionActionSet(XrDeviceBackend &deviceBackend);

        /**
         * @brief Binds all action sets to the device backend.
         *
         * This method binds the created action sets and their associated actions to the OpenXR device backend, allowing the application to receive input events from the XR runtime. It ensures that all action sets are properly registered with the device backend so that input events can be correctly routed to the appropriate handlers for hand tracking, button input, and thumb stick input.
         *
         * @param deviceBackend Reference to the XrDeviceBackend instance that manages the OpenXR device and session, used for binding action sets with the runtime.
         */
        void bindActionSets(XrDeviceBackend &deviceBackend);
    };
} // namespace evan