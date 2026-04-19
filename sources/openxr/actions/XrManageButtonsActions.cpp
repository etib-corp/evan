/*
** ETIB PROJECT, 2026
** evan
** File description:
** XrManageButtonsActions
*/

#include "openxr/actions/XrManageButtonsActions.hpp"
#include "openxr/XrDeviceBackend.hpp"

evan::XrButtonAction::XrButtonAction(utility::event::HandButtonEvent::Button buttonType)
    : _buttonType(buttonType)
{
}

////////////////////
// Public Methods //
////////////////////

std::vector<std::unique_ptr<utility::event::Event>> evan::XrButtonAction::getEvent(evan::XrDeviceBackend &deviceBackend)
{
    std::vector<std::unique_ptr<utility::event::Event>> events = {};

    XrActionStateGetInfo getInfo{XR_TYPE_ACTION_STATE_GET_INFO};
    getInfo.action = getAction();

    XrActionStateBoolean state{XR_TYPE_ACTION_STATE_BOOLEAN};
    xrGetActionStateBoolean(deviceBackend._session, &getInfo, &state);

    if (state.isActive) {
        auto buttonEvent = std::make_unique<utility::event::HandButtonEvent>();
        buttonEvent->setButton(_buttonType);
        buttonEvent->setClicked(state.currentState);

        std::cout << "Button " << static_cast<int>(_buttonType) << " is " << (state.currentState ? "pressed" : "not pressed") << std::endl;

        events.push_back(std::move(buttonEvent));
    }

    return events;
}



evan::XrManageButtonsActions::XrManageButtonsActions(XrActionSet actionSet, XrDeviceBackend &deviceBackend)
{
    _buttonAAction = std::make_unique<XrButtonAction>(utility::event::HandButtonEvent::Button::A);
    _buttonBAction = std::make_unique<XrButtonAction>(utility::event::HandButtonEvent::Button::B);
    _buttonXAction = std::make_unique<XrButtonAction>(utility::event::HandButtonEvent::Button::X);
    _buttonMenuAction = std::make_unique<XrButtonAction>(utility::event::HandButtonEvent::Button::Menu);
    _buttonSystemAction = std::make_unique<XrButtonAction>(utility::event::HandButtonEvent::Button::System);

    evan::AXrAction::PropertiesXrActions propertiesButtonAAction;
    propertiesButtonAAction.actionName = "button_a_action";
    propertiesButtonAAction.bindingPath = "/user/hand/right/input/a/click";
    propertiesButtonAAction.actionSet = actionSet;
    propertiesButtonAAction.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
    _buttonAAction->createAction(propertiesButtonAAction);

    evan::AXrAction::PropertiesXrActions propertiesButtonBAction;
    propertiesButtonBAction.actionName = "button_b_action";
    propertiesButtonBAction.bindingPath = "/user/hand/right/input/b/click";
    propertiesButtonBAction.actionSet = actionSet;
    propertiesButtonBAction.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
    _buttonBAction->createAction(propertiesButtonBAction);

    evan::AXrAction::PropertiesXrActions propertiesButtonXAction;
    propertiesButtonXAction.actionName = "button_x_action";
    propertiesButtonXAction.bindingPath = "/user/hand/left/input/x/click";
    propertiesButtonXAction.actionSet = actionSet;
    propertiesButtonXAction.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
    _buttonXAction->createAction(propertiesButtonXAction);

    evan::AXrAction::PropertiesXrActions propertiesButtonMenuAction;
    propertiesButtonMenuAction.actionName = "button_menu_action";
    propertiesButtonMenuAction.bindingPath = "/user/hand/right/input/menu/click";
    propertiesButtonMenuAction.actionSet = actionSet;
    propertiesButtonMenuAction.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
    _buttonMenuAction->createAction(propertiesButtonMenuAction);

    evan::AXrAction::PropertiesXrActions propertiesButtonSystemAction;
    propertiesButtonSystemAction.actionName = "button_system_action";
    propertiesButtonSystemAction.bindingPath = "/user/hand/right/input/system/click";
    propertiesButtonSystemAction.actionSet = actionSet;
    propertiesButtonSystemAction.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
    _buttonSystemAction->createAction(propertiesButtonSystemAction);
}

evan::XrManageButtonsActions::~XrManageButtonsActions()
{
}

////////////////////
// Public Methods //
////////////////////

std::vector<std::unique_ptr<utility::event::Event>> evan::XrManageButtonsActions::getEvents(evan::XrDeviceBackend &deviceBackend)
{
    std::vector<std::unique_ptr<utility::event::Event>> events;

    auto buttonAEvents = _buttonAAction->getEvent(deviceBackend);
    events.insert(events.end(), std::make_move_iterator(buttonAEvents.begin()), std::make_move_iterator(buttonAEvents.end()));

    auto buttonBEvents = _buttonBAction->getEvent(deviceBackend);
    events.insert(events.end(), std::make_move_iterator(buttonBEvents.begin()), std::make_move_iterator(buttonBEvents.end()));

    auto buttonXEvents = _buttonXAction->getEvent(deviceBackend);
    events.insert(events.end(), std::make_move_iterator(buttonXEvents.begin()), std::make_move_iterator(buttonXEvents.end()));

    auto buttonMenuEvents = _buttonMenuAction->getEvent(deviceBackend);
    events.insert(events.end(), std::make_move_iterator(buttonMenuEvents.begin()), std::make_move_iterator(buttonMenuEvents.end()));

    auto buttonSystemEvents = _buttonSystemAction->getEvent(deviceBackend);
    events.insert(events.end(), std::make_move_iterator(buttonSystemEvents.begin()), std::make_move_iterator(buttonSystemEvents.end()));

    return events;
}