/*
** ETIB PROJECT, 2026
** evan
** File description:
** XrManageButtonsActions
*/

#include "openxr/actions/XrManageButtonsActions.hpp"
#include "openxr/XrDeviceBackend.hpp"

std::vector<std::unique_ptr<utility::event::Event>> evan::XrButtonAAction::getEvent(evan::XrDeviceBackend &deviceBackend)
{
    std::vector<std::unique_ptr<utility::event::Event>> events = {};

    XrActionStateGetInfo getInfo{XR_TYPE_ACTION_STATE_GET_INFO};
    getInfo.action = getAction();

    XrActionStateBoolean state{XR_TYPE_ACTION_STATE_BOOLEAN};
    xrGetActionStateBoolean(deviceBackend._session, &getInfo, &state);

    if (state.isActive) {
        auto buttonEvent = std::make_unique<utility::event::ControllerButtonEvent>();
        buttonEvent->setButton(utility::event::ControllerButtonEvent::Button::A);
        buttonEvent->setClicked(state.currentState);

        std::cout << "Button A is " << (state.currentState ? "pressed" : "released") << std::endl;

        events.push_back(std::move(buttonEvent));
    }

    return events;
}


evan::XrManageButtonsActions::XrManageButtonsActions(XrActionSet actionSet, XrDeviceBackend &deviceBackend)
{
    _buttonAAction = std::make_unique<XrButtonAAction>();

    evan::IXrAction::PropertiesXrActions propertiesButtonAAction;
    propertiesButtonAAction.actionName = "button_a_action";
    propertiesButtonAAction.bindingPath = "/user/hand/right/input/a/click";
    propertiesButtonAAction.actionSet = actionSet;
    propertiesButtonAAction.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
    _buttonAAction->createAction(propertiesButtonAAction);
}

evan::XrManageButtonsActions::~XrManageButtonsActions()
{
}

std::vector<std::unique_ptr<utility::event::Event>> evan::XrManageButtonsActions::getEvents(evan::XrDeviceBackend &deviceBackend)
{
    std::vector<std::unique_ptr<utility::event::Event>> events;

    auto buttonAEvents = _buttonAAction->getEvent(deviceBackend);
    events.insert(events.end(), std::make_move_iterator(buttonAEvents.begin()), std::make_move_iterator(buttonAEvents.end()));

    return events;
}