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

    class XrHandThumbStickAction : public AXrAction {
        public:
            XrHandThumbStickAction(utility::event::HandEvent::HandType handType);

            ~XrHandThumbStickAction();

            std::vector<std::unique_ptr<utility::event::Event>> getEvent(evan::XrDeviceBackend &deviceBackend) override;
        private:
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


            std::unique_ptr<XrHandThumbStickAction> _leftHandThumbStickActions;
            std::unique_ptr<XrHandThumbStickAction> _rightHandThumbStickActions;

        private:

    };
} // namespace evan