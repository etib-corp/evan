/*
** ETIB PROJECT, 2026
** evan
** File description:
** XrHandsMotionActions
*/

#pragma once

#include "evan/EvanPlatform.hpp"

#include "evan/openxr/InteractionProfile.hpp"

#include <utility/event/hand_motion_event.hpp>

#include <utility/logging/loggable.hpp>
#include <utility/logging/default_logger.hpp>

#include <openxr/openxr.h>

namespace evan
{

	class XrDeviceBackend;

	/**
	 * @class XrHandsMotionActions
	 *
	 * @brief The XrHandsMotionActions class is responsible for managing hand
	 * tracking actions in an OpenXR application.
	 *
	 * This class encapsulates the functionality required to define and manage
	 * hand tracking actions, including creating action sets, defining hand
	 * poses, and handling input from hand tracking devices. It provides an
	 * interface for integrating hand tracking capabilities into an OpenXR
	 * application, allowing developers to create immersive experiences that
	 * utilize hand gestures and movements.
	 */
	class XrHandsMotionActions:
		protected utility::logging::Loggable<XrHandsMotionActions,
											 utility::logging::DefaultLogger>
	{
		public:
		/** @brief Constructs an XrHandsMotionActions instance with the
		 * specified action set and device backend.
		 *
		 * This constructor initializes the hand motion actions by creating the
		 * necessary OpenXR actions for hand tracking based on the provided
		 * action set and device backend. It sets up the required action handles
		 * and spaces for hand aim and grip tracking.
		 *
		 * @param actionSet The OpenXR action set to which the hand motion
		 * actions will belong.
		 * @param deviceBackend The device backend used for interacting with the
		 * OpenXR runtime and managing resources.
		 */
		XrHandsMotionActions(XrActionSet actionSet,
							 XrDeviceBackend &deviceBackend);

		/** @brief Destructor for the XrHandsMotionActions class.
		 *
		 * This destructor is responsible for cleaning up any resources
		 * associated with the hand motion actions, such as destroying action
		 * spaces and releasing OpenXR handles. It ensures that all resources
		 * are properly released when an instance of XrHandsMotionActions is
		 * destroyed.
		 */
		~XrHandsMotionActions();

		/** @brief Retrieves the current state of the hand motion actions and
		 * generates corresponding events.
		 *
		 * This method queries the OpenXR runtime for the current state of the
		 * hand motion actions, such as hand poses and gestures. It generates a
		 * vector of events based on the retrieved state, which can be used by
		 * the application to respond to hand tracking input. The events may
		 * include information about hand positions, orientations, and gestures
		 * detected by the OpenXR runtime.
		 *
		 * @param deviceBackend The device backend used for interacting with the
		 * OpenXR runtime and managing resources.
		 * @return A vector of shared pointers to events representing the
		 * current state of the hand motion actions.
		 */
		std::vector<std::shared_ptr<utility::event::Event>>
			getEvents(XrDeviceBackend &deviceBackend);

		/** @brief The XrAction handle for hand aim tracking.
		 *
		 * This member variable holds the XrAction handle that represents the
		 * hand aim tracking action in the OpenXR runtime. It is used to query
		 * the state of hand aim tracking and to generate events based on the
		 * hand's position and orientation.
		 */
		XrAction _handAimAction;
		/** @brief The XrAction handle for hand grip tracking.
		 *
		 * This member variable holds the XrAction handle that represents the
		 * hand grip tracking action in the OpenXR runtime. It is used to query
		 * the state of hand grip tracking and to generate events based on the
		 * hand's position and orientation when gripping.
		 */
		XrAction _handGripAction;

		/** @brief Action spaces for hand aim and grip tracking.
		 *
		 * These member variables hold the XrSpace handles for the hand aim and
		 * grip actions. They are used to define the spatial relationships for
		 * the hand tracking actions, allowing the application to determine the
		 * position and orientation of the hands in the virtual environment
		 * based on the OpenXR runtime's tracking data.
		 */
		XrSpace _handGripSpace[2];

		/** @brief Action spaces for hand aim and grip tracking.
		 *
		 * These member variables hold the XrSpace handles for the hand aim and
		 * grip actions. They are used to define the spatial relationships for
		 * the hand tracking actions, allowing the application to determine the
		 * position and orientation of the hands in the virtual environment
		 * based on the OpenXR runtime's tracking data.
		 */
		XrSpace _handAimSpace[2];

		/** @brief Creates the hand motion actions and their corresponding
		 * spaces.
		 *
		 * This method is responsible for creating the OpenXR actions for hand
		 * aim and grip tracking, as well as the associated action spaces. It
		 * initializes the necessary OpenXR resources to enable hand tracking
		 * functionality in the application.
		 *
		 * @param deviceBackend The device backend used for interacting with the
		 * OpenXR runtime and managing resources.
		 */
		void createHandsMotionSpaces(XrDeviceBackend &deviceBackend);

		/** @brief Shared pointer to the left hand motion event.
		 *
		 * This member variable holds a shared pointer to the HandMotionEvent
		 * representing the current state of the left hand motion action. It is
		 * used to provide context for hand-related actions and can be accessed
		 * by other components of the application that require information about
		 * the left hand's position, orientation, and gestures.
		 */
		std::shared_ptr<utility::event::HandMotionEvent> _leftHandEvent;

		/** @brief Shared pointer to the right hand motion event.
		 *
		 * This member variable holds a shared pointer to the HandMotionEvent
		 * representing the current state of the right hand motion action. It
		 * is used to provide context for hand-related actions and can be
		 * accessed by other components of the application that require
		 * information about the right hand's position, orientation, and
		 * gestures.
		 */
		std::shared_ptr<utility::event::HandMotionEvent> _rightHandEvent;

		private:
		/** @brief Creates the hand motion actions based on the provided action
		 * set and device backend.
		 *
		 * This method initializes the OpenXR actions for hand aim and grip
		 * tracking using the specified action set and device backend. It sets
		 * up the necessary action handles and configurations to enable hand
		 * tracking functionality in the application.
		 *
		 * @param actionSet The OpenXR action set to which the hand motion
		 * actions will belong.
		 * @param deviceBackend The device backend used for interacting with the
		 * OpenXR runtime and managing resources.
		 */
		void createHandsMotionActions(XrActionSet actionSet,
									  XrDeviceBackend &deviceBackend);

		/** @brief Creates a hand motion event based on the current state of
		 * the hand motion actions.
		 *		 * This method queries the OpenXR runtime for the current state
		 * of the hand motion actions and generates a HandMotionEvent that
		 * encapsulates the relevant information. The event can be used by the
		 * application to respond to hand tracking input, such as hand
		 * positions, orientations, and gestures.
		 * @param deviceBackend The device backend used for interacting
		 * with the OpenXR runtime and managing resources.
		 * @param handIndex The index of the hand (0 for left hand, 1 for right
		 * hand) for which the event is being created.
		 * @return A shared pointer to a HandMotionEvent representing the
		 * current state of the specified hand motion action. If the hand motion
		 * action is not available or the event cannot be created, a nullptr is
		 * returned.
		 */
		std::shared_ptr<utility::event::HandMotionEvent>
			createHandMotionEvent(XrDeviceBackend &deviceBackend,
								  int handIndex);
	};
}	 // namespace evan