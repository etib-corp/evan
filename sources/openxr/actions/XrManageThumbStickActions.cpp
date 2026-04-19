/*
** ETIB PROJECT, 2026
** evan
** File description:
** XrManageThumbStickActions
*/

#include "openxr/actions/XrManageThumbStickActions.hpp"
#include "openxr/XrDeviceBackend.hpp"

evan::XrHandThumbStickAction::XrHandThumbStickAction(utility::event::HandEvent::HandType handType)
    : _handType(handType)
{
}

evan::XrHandThumbStickAction::~XrHandThumbStickAction()
{

}

////////////////////
// Public Methods //
////////////////////

std::vector<std::unique_ptr<utility::event::Event>> evan::XrHandThumbStickAction::getEvent(evan::XrDeviceBackend &deviceBackend)
{
    std::vector<std::unique_ptr<utility::event::Event>> events = {};

    XrActionStateGetInfo getInfo{XR_TYPE_ACTION_STATE_GET_INFO};
    getInfo.action = getAction();

    XrActionStateVector2f state{XR_TYPE_ACTION_STATE_VECTOR2F};
    xrGetActionStateVector2f(deviceBackend._session, &getInfo, &state);

    if (state.isActive) {
        auto thumbStickEvent = std::make_unique<utility::event::HandThumbStickEvent>();
        thumbStickEvent->setHandType(_handType);
        thumbStickEvent->setAxis(state.currentState.x, state.currentState.y);

        std::cout << "Thumb stick for hand " << static_cast<int>(_handType) << " is at position (" << state.currentState.x << ", " << state.currentState.y << ")" << std::endl;

        events.push_back(std::move(thumbStickEvent));
    }
    return events;
}

evan::XrManageThumbStickActions::XrManageThumbStickActions(XrActionSet actionSet, XrDeviceBackend &deviceBackend)
{
    _leftHandThumbStickActions = std::make_unique<XrHandThumbStickAction>(utility::event::HandEvent::HandType::Left);
    _rightHandThumbStickActions = std::make_unique<XrHandThumbStickAction>(utility::event::HandEvent::HandType::Right);

    // Create actions for left hand thumb stick
    evan::AXrAction::PropertiesXrActions propertiesLeftThumbStickAction;
    propertiesLeftThumbStickAction.actionName = "left_thumb_stick_action";
    propertiesLeftThumbStickAction.bindingPath = "/user/hand/left/input/thumbstick";
    propertiesLeftThumbStickAction.actionSet = actionSet;
    propertiesLeftThumbStickAction.actionType = XR_ACTION_TYPE_VECTOR2F_INPUT;
    _leftHandThumbStickActions->createAction(propertiesLeftThumbStickAction);

    // Create actions for right hand thumb stick
    evan::AXrAction::PropertiesXrActions propertiesRightThumbStickAction;
    propertiesRightThumbStickAction.actionName = "right_thumb_stick_action";
    propertiesRightThumbStickAction.bindingPath = "/user/hand/right/input/thumbstick";
    propertiesRightThumbStickAction.actionSet = actionSet;
    propertiesRightThumbStickAction.actionType = XR_ACTION_TYPE_VECTOR2F_INPUT;
    _rightHandThumbStickActions->createAction(propertiesRightThumbStickAction);
}

evan::XrManageThumbStickActions::~XrManageThumbStickActions()
{

}

////////////////////
// Public Methods //
////////////////////

std::vector<std::unique_ptr<utility::event::Event>> evan::XrManageThumbStickActions::getEvents(evan::XrDeviceBackend &deviceBackend)
{
    std::vector<std::unique_ptr<utility::event::Event>> events;

    // Get events from left hand thumb stick actions
    auto leftHandEvents = _leftHandThumbStickActions->getEvent(deviceBackend);
    events.insert(events.end(), std::make_move_iterator(leftHandEvents.begin()), std::make_move_iterator(leftHandEvents.end()));

    // Get events from right hand thumb stick actions
    auto rightHandEvents = _rightHandThumbStickActions->getEvent(deviceBackend);
    events.insert(events.end(), std::make_move_iterator(rightHandEvents.begin()), std::make_move_iterator(rightHandEvents.end()));

    return events;
}

/////////////////////
// Private Methods //
/////////////////////
