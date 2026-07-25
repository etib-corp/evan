/*
** ETIB PROJECT, 2026
** evan
** File description:
** XrManageThumbStickActions
*/

#include "evan/openxr/actions/XrManageThumbStickActions.hpp"
#include "evan/openxr/XrDeviceBackend.hpp"

evan::XrHandThumbStickAction::XrHandThumbStickAction(
	utility::event::HandEvent::HandType handType)
	: _handType(handType)
{
	this->getLogger().info() << "Creating thumb stick action for hand "
							 << static_cast<int>(handType) << "...";
}

evan::XrHandThumbStickAction::~XrHandThumbStickAction()
{
	this->getLogger().info() << "Destroying thumb stick action for hand "
							 << static_cast<int>(_handType) << "...";
}

////////////////////
// Public Methods //
////////////////////

std::vector<std::shared_ptr<utility::event::Event>>
	evan::XrHandThumbStickAction::getEvent(
		evan::XrDeviceBackend &deviceBackend,
		std::shared_ptr<utility::event::HandMotionEvent> handMotionEvent)
{
	std::vector<std::shared_ptr<utility::event::Event>> events = {};

	XrActionStateGetInfo getInfo { XR_TYPE_ACTION_STATE_GET_INFO };
	getInfo.action = getAction();

	XrActionStateVector2f state { XR_TYPE_ACTION_STATE_VECTOR2F };
	xrGetActionStateVector2f(deviceBackend._session, &getInfo, &state);

	if (state.isActive) {
		auto thumbStickEvent =
			std::make_shared<utility::event::HandThumbStickEvent>();
		thumbStickEvent->setHandType(_handType);
		thumbStickEvent->setAxis(state.currentState.x, state.currentState.y);

		this->getLogger().info()
			<< "Thumb stick for hand " << static_cast<int>(_handType)
			<< " is active at position (" + std::to_string(state.currentState.x)
				+ ", " + std::to_string(state.currentState.y) + ")";
		events.push_back(std::move(thumbStickEvent));
	}
	return events;
}

evan::XrManageThumbStickActions::XrManageThumbStickActions(
	XrActionSet actionSet, XrDeviceBackend &deviceBackend)
{
	this->getLogger().info()
		<< "Creating thumb stick actions for left and right hands...";

	_leftHandThumbStickActions = std::make_unique<XrHandThumbStickAction>(
		utility::event::HandEvent::HandType::Left);
	_rightHandThumbStickActions = std::make_unique<XrHandThumbStickAction>(
		utility::event::HandEvent::HandType::Right);

	// Create actions for left hand thumb stick
	evan::AXrAction::PropertiesXrActions propertiesLeftThumbStickAction;
	propertiesLeftThumbStickAction.actionName = "left_thumb_stick_action";
	propertiesLeftThumbStickAction.bindingPath =
		"/user/hand/left/input/thumbstick";
	propertiesLeftThumbStickAction.actionSet  = actionSet;
	propertiesLeftThumbStickAction.actionType = XR_ACTION_TYPE_VECTOR2F_INPUT;
	_leftHandThumbStickActions->createAction(propertiesLeftThumbStickAction);

	this->getLogger().info()
		<< "Left hand thumb stick action created successfully.";

	// Create actions for right hand thumb stick
	evan::AXrAction::PropertiesXrActions propertiesRightThumbStickAction;
	propertiesRightThumbStickAction.actionName = "right_thumb_stick_action";
	propertiesRightThumbStickAction.bindingPath =
		"/user/hand/right/input/thumbstick";
	propertiesRightThumbStickAction.actionSet  = actionSet;
	propertiesRightThumbStickAction.actionType = XR_ACTION_TYPE_VECTOR2F_INPUT;
	_rightHandThumbStickActions->createAction(propertiesRightThumbStickAction);

	this->getLogger().info()
		<< "Right hand thumb stick action created successfully.";
}

evan::XrManageThumbStickActions::~XrManageThumbStickActions()
{
	this->getLogger().info() << "Destroying thumb stick actions...";
}

////////////////////
// Public Methods //
////////////////////

std::vector<std::shared_ptr<utility::event::Event>>
	evan::XrManageThumbStickActions::getEvents(
		evan::XrDeviceBackend &deviceBackend)
{
	std::vector<std::shared_ptr<utility::event::Event>> events;

	// Get events from left hand thumb stick actions
	auto leftHandEvents =
		_leftHandThumbStickActions->getEvent(deviceBackend, nullptr);
	events.insert(events.end(), std::make_move_iterator(leftHandEvents.begin()),
				  std::make_move_iterator(leftHandEvents.end()));

	// Get events from right hand thumb stick actions
	auto rightHandEvents =
		_rightHandThumbStickActions->getEvent(deviceBackend, nullptr);
	events.insert(events.end(),
				  std::make_move_iterator(rightHandEvents.begin()),
				  std::make_move_iterator(rightHandEvents.end()));

	return events;
}

/////////////////////
// Private Methods //
/////////////////////
