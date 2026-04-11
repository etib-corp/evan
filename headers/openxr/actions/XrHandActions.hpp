/*
** ETIB PROJECT, 2026
** evan
** File description:
** XrHandActions
*/

#pragma once

#include "EvanPlatform.hpp"

#include "openxr/InteractionProfile.hpp"

#ifdef None
#undef None
#endif

#include <utility/event/controller_motion_event.hpp>
#include <utility/event/controller_button_event.hpp>
#include <utility/event/controller_thumb_stick_event.hpp>

#include <openxr/openxr.h>

namespace evan {

    class XrDeviceBackend;

    /**
     * @brief The XrHandActions class is responsible for managing hand tracking
     * actions in an OpenXR application.
     *
     * This class encapsulates the functionality required to define and manage
     * hand tracking actions, including creating action sets, defining hand
     * poses, and handling input from hand tracking devices. It provides an
     * interface for integrating hand tracking capabilities into an OpenXR
     * application, allowing developers to create immersive experiences that
     * utilize hand gestures and movements.
     */
    class XrHandActions {
        public:
            enum class HandType {
                Left,
                Right
            };
            XrHandActions(HandType handType, XrActionSet actionSet, XrDeviceBackend &deviceBackend);

            ~XrHandActions();

            std::vector<std::shared_ptr<utility::event::Event>> getEvents(XrDeviceBackend &deviceBackend);

        private:
            std::string _handPath; // Path for the hand (e.g., "/user/hand/left" or "/user/hand/right")

            std::string _prefixActionName; // Prefix for action names (e.g., "left_hand" or "right_hand")

            XrAction _handAction; // Action for hand aim tracking
            XrAction _handGripAction; // Action for hand grip tracking

            XrPath _handXrPath; // Path for the hand aim action
            XrPath _handGripPath; // Path for the hand grip action

            XrSpace _handActionSpace; // Action space for the hand aim action

    };
} // namespace evan