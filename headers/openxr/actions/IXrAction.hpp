/*
** ETIB PROJECT, 2026
** evan
** File description:
** IXrAction
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

    class IXrAction {
        public:

            struct PropertiesXrActions {
                std::string actionName;
                std::string bindingPath;
                XrActionSet actionSet;
                XrActionType actionType;
            };

            virtual ~IXrAction()
            {
                xrDestroyAction(_action);
                if (_actionSpace != XR_NULL_HANDLE) {
                    xrDestroySpace(_actionSpace);
                }
            }

            virtual std::vector<std::unique_ptr<utility::event::Event>> getEvent(XrDeviceBackend &deviceBackend) = 0;

            XrAction getAction() const;

            const std::string &getActionName() const;

            const std::string &getBindingPath() const;

            void createAction(const PropertiesXrActions &properties);

        private:
            XrAction _action;
            XrSpace _actionSpace = XR_NULL_HANDLE;
            XrPath _actionSubactionPath;

            std::string _actionName;

            std::string _bindingPath;
    };

}