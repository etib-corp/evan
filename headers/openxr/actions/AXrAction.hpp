/*
** ETIB PROJECT, 2026
** evan
** File description:
** AXrAction
*/

#pragma once

#include "EvanPlatform.hpp"

#include "openxr/InteractionProfile.hpp"

#ifdef None
#undef None
#endif

#include <utility/event/controller_button_event.hpp>


namespace evan {

    class XrDeviceBackend;

    /**
     * @class AXrAction
     *
     * @brief Represents an OpenXR action and its associated properties.
     *
     * This class encapsulates the creation and management of an OpenXR action,
     * including its name, binding path, and the underlying XrAction handle. It
     * provides methods to retrieve the action's properties and to create the
     * action based on specified properties. The class also ensures proper
     * cleanup of the OpenXR resources when the object is destroyed.
     */
    class AXrAction {
        public:

            /**
             * @struct PropertiesXrActions
             * @brief Encapsulates properties required for creating an OpenXR action.
             *
             * This structure holds all necessary parameters for creating an OpenXR
             * action, including the action's name, binding path, associated action
             * set, and the type of action. It serves as a convenient way to pass
             * all relevant information needed to create an OpenXR action in a
             * single object.
             */
            struct PropertiesXrActions {
                /** @brief The name of the action, used for identification and debugging
                 * purposes. It should be unique within the action set.
                 */
                std::string actionName;
                /** @brief The binding path for the action, specifying how the action
                 * is bound to input devices. This path should follow OpenXR's
                 * binding syntax and conventions.
                 */
                std::string bindingPath;
                /** @brief The action set to which this action belongs. An action set
                 * is a collection of actions that can be activated or deactivated
                 * together, allowing for better organization and management of
                 * actions in an OpenXR application.
                 */
                XrActionSet actionSet;
                /** @brief The type of the action, indicating how it is used in the
                 * application. This can be a boolean input, a float input, a pose
                 * input, etc., and determines how the action is processed and
                 * interpreted by the OpenXR runtime.
                 */
                XrActionType actionType;
            };

            /** @brief Destructor for AXrAction, responsible for cleaning up OpenXR resources.
             *
             * This destructor ensures that the XrAction handle is properly destroyed
             * when the AXrAction object goes out of scope. It also checks if the
             * associated action space is valid and destroys it if necessary, ensuring
             * that all OpenXR resources are released correctly to prevent memory leaks
             * and other issues.
             */
            virtual ~AXrAction()
            {
                xrDestroyAction(_action);
                if (_actionSpace != XR_NULL_HANDLE) {
                    xrDestroySpace(_actionSpace);
                }
            }

            /** @brief Retrieves the XrAction handle associated with this AXrAction.
             *
             * This method returns the XrAction handle that represents the OpenXR
             * action managed by this class. The handle can be used in OpenXR
             * API calls to interact with the action, such as binding it to input devices or querying its state.
             * @return XrAction The handle of the OpenXR action.
             */
            virtual std::vector<std::unique_ptr<utility::event::Event>> getEvent(XrDeviceBackend &deviceBackend) = 0;

            /** @brief Retrieves the XrAction handle associated with this AXrAction.
             *
             * This method returns the XrAction handle that represents the OpenXR
             * action managed by this class. The handle can be used in OpenXR
             * API calls to interact with the action, such as binding it to input devices or querying its state.
             * @return XrAction The handle of the OpenXR action.
             */
            XrAction getAction() const;

            /** @brief Retrieves the name of the action.
             *
             * This method returns the name of the action, which is used for
             * identification and debugging purposes. The name should be unique within
             * the action set to avoid conflicts with other actions.
             *
             * @return const std::string& The name of the action.
             */
            const std::string &getActionName() const;

            /** @brief Retrieves the binding path of the action.
             *
             * This method returns the binding path for the action, which specifies
             * how the action is bound to input devices. The binding path should
             * follow OpenXR's binding syntax and conventions to ensure proper
             * functionality when interacting with input devices.
             *
             * @return const std::string& The binding path of the action.
             */
            const std::string &getBindingPath() const;

            /** @brief Creates an OpenXR action based on the provided properties.
             *
             * This method takes a PropertiesXrActions structure containing all
             * necessary parameters for creating an OpenXR action, such as the
             * action's name, binding path, associated action set, and type. It
             * uses this information to create the action using the OpenXR API and
             * stores the resulting XrAction handle for later use. If the action
             * creation fails, it throws a runtime error with details about the failure.
             *
             * @param properties A structure containing all necessary parameters for
             * creating an OpenXR action.
             *
             * @throws std::runtime_error If action creation fails, providing details
             * about the failure.
             */
            void createAction(const PropertiesXrActions &properties);

        private:
            /** @brief The XrAction handle representing the OpenXR action managed by this class. */
            XrAction _action;

            /** @brief The XrSpace handle associated with the action, used for spatial interactions. */
            XrSpace _actionSpace = XR_NULL_HANDLE;

            /** @brief The subaction path for the action, specifying the input source or device associated with the action. */
            XrPath _actionSubactionPath;

            /** @brief The name of the action, used for identification and debugging purposes. */
            std::string _actionName;

            /** @brief The binding path for the action, specifying how the action is bound to input devices. */
            std::string _bindingPath;
    };

}