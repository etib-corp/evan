/*
** ETIB PROJECT, 2026
** evan
** File description:
** XrManageButtonsActions
*/

#pragma once

#include "EvanPlatform.hpp"

#include "openxr/actions/AXrAction.hpp"
#include "openxr/InteractionProfile.hpp"

#ifdef None
#undef None
#endif

#include <utility/event/controller_button_event.hpp>


namespace evan {

    class XrDeviceBackend;

    /**
     * @class XrManageButtonsActions
     *
     * @brief Manages button actions for an OpenXR application.
     *
     * This class is responsible for creating and managing button actions,
     * such as the A button on a controller, within an OpenXR application.
     * It encapsulates the functionality required to define button actions, query their state,
     * and generate corresponding events based on user input.
     * The class interacts with the OpenXR runtime through the provided device backend to retrieve
     * action states and create events that can be used by the application to respond to button presses and releases.
     *
     * @note This class is designed to be extended with additional button actions as needed,
     * allowing for flexible management of various input actions in an OpenXR application.
     */
    class XrButtonAAction: public AXrAction {
        public:
            /** @brief Default constructor for the XrButtonAAction class.
             *
            */
            XrButtonAAction() = default;

            /** @brief Destructor for the XrButtonAAction class.
             *
            */
            ~XrButtonAAction() = default;

            /** @brief Retrieves the current state of the A button action and generates corresponding events.
             *
             * This method queries the OpenXR runtime for the current state of the A button action, such as whether it is pressed or released.
             * It generates a vector of events based on the retrieved state, which can be used by the application to respond to button input.
             * The events may include information about whether the A button is currently pressed or released, allowing the application to react accordingly.
             *
             * @param deviceBackend The device backend used for interacting with the OpenXR runtime and managing resources.
             * @return A vector of unique pointers to events representing the current state of the A button action.
             */
            std::vector<std::unique_ptr<utility::event::Event>> getEvent(XrDeviceBackend &deviceBackend) override;

        private:

    };

    /**
     * @class XrManageButtonsActions
     *
     * @brief The XrManageButtonsActions class is responsible for managing button actions in an OpenXR application.
     * This class encapsulates the functionality required to define and manage button actions, including creating action sets,
     * defining button actions, and handling input from button devices. It provides an interface for integrating
     * button input capabilities into an OpenXR application, allowing developers to create immersive experiences that
     * utilize button presses and releases.
     *
     * @note This class is designed to be extended with additional button actions as needed, allowing for flexible management
     * of various input actions in an OpenXR application.
     */
    class XrManageButtonsActions {
        public:

            /** @brief Constructs an XrManageButtonsActions instance with the specified action set and device backend.
            *
            * This constructor initializes the button actions by creating the necessary
            * OpenXR actions for button input based on the provided action set and device backend.
            * It sets up the required action handles for button tracking and input management.
            *
            * @param actionSet The OpenXR action set to which the button actions will belong.
            * @param deviceBackend The device backend used for interacting with the OpenXR runtime and managing resources.
            */
            XrManageButtonsActions(XrActionSet actionSet, XrDeviceBackend &deviceBackend);

            /** @brief Destructor for the XrManageButtonsActions class.
             * This destructor is responsible for cleaning up any resources associated with the button actions,
             * such as destroying action spaces and releasing OpenXR handles. It ensures that all resources are
             * properly released when an instance of XrManageButtonsActions is destroyed.
             */
            ~XrManageButtonsActions();

            /** @brief Retrieves the current state of the button actions and generates corresponding events.
             *
             * This method queries the OpenXR runtime for the current state of the button actions, such as
             * whether specific buttons are pressed or released. It generates a vector of events based on the retrieved state, which can
             * be used by the application to respond to button input. The events may include information about whether specific buttons are currently pressed or released, allowing the application to react accordingly.
             *
             * @param deviceBackend The device backend used for interacting with the OpenXR runtime and managing resources.
             * @return A vector of unique pointers to events representing the current state of the button actions.
             */
            std::vector<std::unique_ptr<utility::event::Event>> getEvents(XrDeviceBackend &deviceBackend);

            /** @brief The XrButtonAAction instance for managing the A button action.
            *
            * This member variable holds an instance of the XrButtonAAction class, which is responsible for managing the state and events related to the A button action in the OpenXR runtime. It allows the XrManageButtonsActions class to query the state of the A button and generate corresponding events based on user input.
            */
            std::unique_ptr<XrButtonAAction> _buttonAAction;
        private:

    };

}