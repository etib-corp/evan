/*
** ETIB PROJECT, 2026
** evan
** File description:
** XrManageButtonsActions
*/

#include "evan/openxr/actions/XrManageButtonsActions.hpp"
#include "evan/openxr/XrDeviceBackend.hpp"

evan::XrButtonAction::XrButtonAction(
	utility::event::HandButtonEvent::Button buttonType)
	: _buttonType(buttonType)
{
	this->getLogger().info() << "Creating button action for button: "
							 << static_cast<int>(_buttonType);
}

evan::XrManageButtonsActions::~XrManageButtonsActions()
{
	this->getLogger().info() << "Destroying button actions...";
}

////////////////////
// Public Methods //
////////////////////

std::vector<std::shared_ptr<utility::event::Event>>
	evan::XrButtonAction::getEvent(
		evan::XrDeviceBackend &deviceBackend,
		std::shared_ptr<utility::event::HandMotionEvent> handMotionEvent)
{
	std::vector<std::shared_ptr<utility::event::Event>> events = {};

	XrActionStateGetInfo getInfo { XR_TYPE_ACTION_STATE_GET_INFO };
	getInfo.action = getAction();

	XrActionStateBoolean state { XR_TYPE_ACTION_STATE_BOOLEAN };
	xrGetActionStateBoolean(deviceBackend._session, &getInfo, &state);

	if (state.isActive) {
		if (state.currentState != _lastState) {
			_lastState = state.currentState;
		} else {
			return events;
		}

		auto buttonEvent = std::make_shared<utility::event::HandButtonEvent>();
		buttonEvent->setButton(_buttonType);
		buttonEvent->setButtonPressed(state.currentState);
		if (handMotionEvent) {
			buttonEvent->setAim(handMotionEvent->getAim());
			buttonEvent->setGrip(handMotionEvent->getGrip());
		}

		this->getLogger().info()
			<< "Button " << static_cast<int>(_buttonType) << " is "
			<< (state.currentState ? "pressed" : "not pressed")
			<< " at the pose: " << buttonEvent->getGrip();

		events.push_back(std::move(buttonEvent));
	}

	return events;
}

evan::XrManageButtonsActions::XrManageButtonsActions(
	XrActionSet actionSet, XrDeviceBackend &deviceBackend)
{
	this->getLogger().info() << "Creating button actions...";

	_buttonAAction = std::make_unique<XrButtonAction>(
		utility::event::HandButtonEvent::Button::A);
	_buttonBAction = std::make_unique<XrButtonAction>(
		utility::event::HandButtonEvent::Button::B);
	_buttonXAction = std::make_unique<XrButtonAction>(
		utility::event::HandButtonEvent::Button::X);
	_buttonYAction = std::make_unique<XrButtonAction>(
		utility::event::HandButtonEvent::Button::Y);
	_buttonMenuAction = std::make_unique<XrButtonAction>(
		utility::event::HandButtonEvent::Button::Menu);
	_buttonSystemAction = std::make_unique<XrButtonAction>(
		utility::event::HandButtonEvent::Button::System);

	evan::AXrAction::PropertiesXrActions propertiesButtonAAction;
	propertiesButtonAAction.actionName	= "button_a_action";
	propertiesButtonAAction.bindingPath = "/user/hand/right/input/a/click";
	propertiesButtonAAction.actionSet	= actionSet;
	propertiesButtonAAction.actionType	= XR_ACTION_TYPE_BOOLEAN_INPUT;
	_buttonAAction->createAction(propertiesButtonAAction);

	this->getLogger().info()
		<< "Button A action created with name: "
		<< propertiesButtonAAction.actionName
		<< " and binding path: " << propertiesButtonAAction.bindingPath;

	evan::AXrAction::PropertiesXrActions propertiesButtonBAction;
	propertiesButtonBAction.actionName	= "button_b_action";
	propertiesButtonBAction.bindingPath = "/user/hand/right/input/b/click";
	propertiesButtonBAction.actionSet	= actionSet;
	propertiesButtonBAction.actionType	= XR_ACTION_TYPE_BOOLEAN_INPUT;
	_buttonBAction->createAction(propertiesButtonBAction);

	this->getLogger().info()
		<< "Button B action created with name: "
		<< propertiesButtonBAction.actionName
		<< " and binding path: " << propertiesButtonBAction.bindingPath;

	evan::AXrAction::PropertiesXrActions propertiesButtonXAction;
	propertiesButtonXAction.actionName	= "button_x_action";
	propertiesButtonXAction.bindingPath = "/user/hand/left/input/x/click";
	propertiesButtonXAction.actionSet	= actionSet;
	propertiesButtonXAction.actionType	= XR_ACTION_TYPE_BOOLEAN_INPUT;
	_buttonXAction->createAction(propertiesButtonXAction);

	this->getLogger().info()
		<< "Button X action created with name: "
		<< propertiesButtonXAction.actionName
		<< " and binding path: " << propertiesButtonXAction.bindingPath;

	evan::AXrAction::PropertiesXrActions propertiesButtonYAction;
	propertiesButtonYAction.actionName	= "button_y_action";
	propertiesButtonYAction.bindingPath = "/user/hand/left/input/y/click";
	propertiesButtonYAction.actionSet	= actionSet;
	propertiesButtonYAction.actionType	= XR_ACTION_TYPE_BOOLEAN_INPUT;
	_buttonYAction->createAction(propertiesButtonYAction);

	this->getLogger().info()
		<< "Button Y action created with name: "
		<< propertiesButtonYAction.actionName
		<< " and binding path: " << propertiesButtonYAction.bindingPath;

	evan::AXrAction::PropertiesXrActions propertiesButtonMenuAction;
	propertiesButtonMenuAction.actionName = "button_menu_action";
	propertiesButtonMenuAction.bindingPath =
		"/user/hand/right/input/menu/click";
	propertiesButtonMenuAction.actionSet  = actionSet;
	propertiesButtonMenuAction.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
	_buttonMenuAction->createAction(propertiesButtonMenuAction);

	this->getLogger().info()
		<< "Button Menu action created with name: "
		<< propertiesButtonMenuAction.actionName
		<< " and binding path: " << propertiesButtonMenuAction.bindingPath;

	evan::AXrAction::PropertiesXrActions propertiesButtonSystemAction;
	propertiesButtonSystemAction.actionName = "button_system_action";
	propertiesButtonSystemAction.bindingPath =
		"/user/hand/right/input/system/click";
	propertiesButtonSystemAction.actionSet	= actionSet;
	propertiesButtonSystemAction.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
	_buttonSystemAction->createAction(propertiesButtonSystemAction);

	this->getLogger().info()
		<< "Button System action created with name: "
		<< propertiesButtonSystemAction.actionName
		<< " and binding path: " << propertiesButtonSystemAction.bindingPath;
}

std::vector<std::shared_ptr<utility::event::Event>>
	evan::XrManageButtonsActions::getEvents(
		evan::XrDeviceBackend &deviceBackend,
		std::shared_ptr<utility::event::HandMotionEvent> handMotionLeftEvent,
		std::shared_ptr<utility::event::HandMotionEvent> handMotionRightEvent)
{
	std::vector<std::shared_ptr<utility::event::Event>> events;

	auto buttonAEvents =
		_buttonAAction->getEvent(deviceBackend, handMotionRightEvent);
	events.insert(events.end(), std::make_move_iterator(buttonAEvents.begin()),
				  std::make_move_iterator(buttonAEvents.end()));

	auto buttonBEvents =
		_buttonBAction->getEvent(deviceBackend, handMotionRightEvent);
	events.insert(events.end(), std::make_move_iterator(buttonBEvents.begin()),
				  std::make_move_iterator(buttonBEvents.end()));

	auto buttonXEvents =
		_buttonXAction->getEvent(deviceBackend, handMotionLeftEvent);
	events.insert(events.end(), std::make_move_iterator(buttonXEvents.begin()),
				  std::make_move_iterator(buttonXEvents.end()));

	auto buttonYEvents =
		_buttonYAction->getEvent(deviceBackend, handMotionLeftEvent);
	events.insert(events.end(), std::make_move_iterator(buttonYEvents.begin()),
				  std::make_move_iterator(buttonYEvents.end()));

	auto buttonMenuEvents =
		_buttonMenuAction->getEvent(deviceBackend, handMotionLeftEvent);
	events.insert(events.end(),
				  std::make_move_iterator(buttonMenuEvents.begin()),
				  std::make_move_iterator(buttonMenuEvents.end()));

	auto buttonSystemEvents =
		_buttonSystemAction->getEvent(deviceBackend, handMotionRightEvent);
	events.insert(events.end(),
				  std::make_move_iterator(buttonSystemEvents.begin()),
				  std::make_move_iterator(buttonSystemEvents.end()));

	return events;
}