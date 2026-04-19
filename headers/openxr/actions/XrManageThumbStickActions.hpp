    /*
** ETIB PROJECT, 2026
** evan
** File description:
** XrManageThumbStickActions
*/

#pragma once

#include "EvanPlatform.hpp"

#include "openxr/actions/AXrAction.hpp"
#include "openxr/InteractionProfile.hpp"

#include <utility/event/hand_thumb_stick_event.hpp>

namespace evan {

    class XrDeviceBackend;

    /** @class XrHandThumbStickAction
     * @brief Represents an OpenXR action for thumb stick input from hand tracking devices.
     *
     * This class encapsulates the functionality required to define and manage a thumb stick action for hand tracking devices in an OpenXR application. It provides an interface for retrieving thumb stick events based on the current state of the thumb stick input detected by the OpenXR runtime.
     */
    class XrHandThumbStickAction : public AXrAction {
        public:
            /** @brief Constructs an XrHandThumbStickAction instance for the specified hand type.
             *
             * This constructor initializes the thumb stick action for a specific hand (left or right) by creating the necessary OpenXR action and setting up the required resources for tracking thumb stick input from hand tracking devices.
             *
             * @param handType The type of hand (left or right) for which to create the thumb stick action.
             */
            XrHandThumbStickAction(utility::event::HandEvent::HandType handType);

            /** @brief Destructor for the XrHandThumbStickAction class.
             *
             * This destructor is responsible for cleaning up any resources associated with the thumb stick action, such as destroying action spaces and releasing OpenXR handles. It ensures that all resources are properly released when an instance of XrHandThumbStickAction is destroyed.
             */
            ~XrHandThumbStickAction();

            /** @brief Retrieves the current thumb stick events from the device backend.
             *
             * This method queries the device backend for the current state of thumb stick input and generates a list of events based on the thumb stick movements and gestures detected. It returns a vector of unique pointers to Event objects representing the thumb stick events that have occurred since the last query.
             *
             * @param deviceBackend The device backend used for interacting with the OpenXR runtime and managing resources.
             * @return A vector of unique pointers to Event objects representing the thumb stick events.
             */
            std::vector<std::unique_ptr<utility::event::Event>> getEvent(evan::XrDeviceBackend &deviceBackend) override;
        private:

            /**
             * @brief The type of hand (left or right) associated with this thumb stick action.
             * This member variable indicates which hand's thumb stick input is being tracked by this action, allowing the application to differentiate between left and right hand thumb stick events when processing input from the OpenXR runtime.
             * It is initialized during the construction of the XrHandThumbStickAction class and is used
             * to determine which hand's thumb stick input to query when retrieving events from the device backend.
             */
            utility::event::HandEvent::HandType _handType;
    };

    /**
     * @class XrManageThumbStickActions
     *
     * @brief The XrManageThumbStickActions class is responsible for managing thumb stick input from hand tracking devices in an OpenXR application.
     *
     * This class encapsulates the functionality required to define and manage thumb stick input actions for hand tracking devices. It provides an interface for integrating thumb stick input capabilities into an OpenXR application, allowing developers to create immersive experiences that utilize thumb stick gestures and movements from hand tracking devices.
     */
    class XrManageThumbStickActions {
        public:

            /** @brief Constructs an XrManageThumbStickActions instance with the specified action set and device backend.
             *
             * This constructor initializes the thumb stick actions by creating the necessary OpenXR actions for thumb stick input based on the provided action set and device backend. It sets up the required action handles and spaces for thumb stick tracking.
             *
             * @param actionSet The OpenXR action set to which the thumb stick actions will belong.
             * @param deviceBackend The device backend used for interacting with the OpenXR runtime and managing resources.
             */
            XrManageThumbStickActions(XrActionSet actionSet, XrDeviceBackend &deviceBackend);

            /** @brief Destructor for the XrManageThumbStickActions class.
             *
             * This destructor is responsible for cleaning up any resources associated with the thumb stick actions, such as destroying action spaces and releasing OpenXR handles. It ensures that all resources are properly released when an instance of XrManageThumbStickActions is destroyed.
             */
            ~XrManageThumbStickActions();

            /** @brief Retrieves the current thumb stick events from the device backend.
             *
             * This method queries the device backend for the current state of thumb stick input and generates a list of events based on the thumb stick movements and gestures detected. It returns a vector of unique pointers to Event objects representing the thumb stick events that have occurred since the last query.
             *
             * @param deviceBackend The device backend used for interacting with the OpenXR runtime and managing resources.
             * @return A vector of unique pointers to Event objects representing the thumb stick events.
             */
            std::vector<std::unique_ptr<utility::event::Event>> getEvents(evan::XrDeviceBackend &deviceBackend);

            /**
             * @brief Unique pointer to the action associated with the left hand thumb stick.
             *
             * This member holds the action object responsible for handling input events related to the left hand thumb stick.
             * It is initialized during the construction of the XrManageThumbStickActions class.
             */
            std::unique_ptr<XrHandThumbStickAction> _leftHandThumbStickActions;

            /**
             * @brief Unique pointer to the action associated with the right hand thumb stick.
             *
             * This member holds the action object responsible for handling input events related to the right hand thumb stick.
             * It is initialized during the construction of the XrManageThumbStickActions class.
             */
            std::unique_ptr<XrHandThumbStickAction> _rightHandThumbStickActions;

        private:

    };
} // namespace evan