/*
** ETIB PROJECT, 2026
** evan
** File description:
** XrHandsMotionActions
*/

#pragma once

#include "EvanPlatform.hpp"

#include "openxr/InteractionProfile.hpp"

#ifdef None
#undef None
#endif

#include <utility/event/controller_motion_event.hpp>
#include <utility/event/controller_thumb_stick_event.hpp>

#include <openxr/openxr.h>

namespace evan {

    class XrDeviceBackend;

    /**
     * @brief The XrHandsMotionActions class is responsible for managing hand tracking
     * actions in an OpenXR application.
     *
     * This class encapsulates the functionality required to define and manage
     * hand tracking actions, including creating action sets, defining hand
     * poses, and handling input from hand tracking devices. It provides an
     * interface for integrating hand tracking capabilities into an OpenXR
     * application, allowing developers to create immersive experiences that
     * utilize hand gestures and movements.
     */
    class XrHandsMotionActions {
        public:

            XrHandsMotionActions(XrActionSet actionSet, XrDeviceBackend &deviceBackend);

            ~XrHandsMotionActions();

            std::vector<std::unique_ptr<utility::event::Event>> getEvents(XrDeviceBackend &deviceBackend);

            XrAction _handAimAction; // Action for hand aim tracking
            XrAction _handGripAction; // Action for hand grip tracking

            // XrPath _handGripPath[2]; // Path for the hand grip action
            XrSpace _handGripSpace[2]; // Action space for the hand grip action

            // XrPath _handAimSubactionPath[2];
            XrSpace _handAimSpace[2];

            XrPath _handActionSubactionPath[2]; // Subaction paths for hand actions

            void createHandsMotionSpaces(XrDeviceBackend &deviceBackend);

        private:
            void createHandsMotionActions(XrActionSet actionSet, XrDeviceBackend &deviceBackend);



    };
} // namespace evan