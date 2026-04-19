/*
** ETIB PROJECT, 2026
** evan
** File description:
** XrManageButtonsActions
*/

#pragma once

#include "EvanPlatform.hpp"

#include "openxr/actions/IXrAction.hpp"
#include "openxr/InteractionProfile.hpp"

#ifdef None
#undef None
#endif

#include <utility/event/controller_button_event.hpp>


namespace evan {

    class XrDeviceBackend;

    class XrButtonAAction: public IXrAction {
        public:
            XrButtonAAction() = default;

            ~XrButtonAAction() = default;

            std::vector<std::unique_ptr<utility::event::Event>> getEvent(XrDeviceBackend &deviceBackend) override;

        private:

    };

    class XrManageButtonsActions {
        public:

            XrManageButtonsActions(XrActionSet actionSet, XrDeviceBackend &deviceBackend);

            ~XrManageButtonsActions();

            std::vector<std::unique_ptr<utility::event::Event>> getEvents(XrDeviceBackend &deviceBackend);

            std::unique_ptr<XrButtonAAction> _buttonAAction;
        private:

    };

}