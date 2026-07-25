/*
** ETIB PROJECT, 2026
** evan
** File description:
** XrManageActions
*/

#include "evan/openxr/XrManageActions.hpp"
#include "evan/openxr/XrDeviceBackend.hpp"

evan::XrManageActions::XrManageActions(XrDeviceBackend &deviceBackend)
{
	this->getLogger().info() << "Initializing XrManageActions";

	createActionSet(deviceBackend);

	_handsMotionActions =
		std::make_unique<XrHandsMotionActions>(_actionSet, deviceBackend);
	_manageButtonsActions =
		std::make_unique<XrManageButtonsActions>(_actionSet, deviceBackend);
	_manageThumbStickActions =
		std::make_unique<XrManageThumbStickActions>(_actionSet, deviceBackend);

	bindActionSets(deviceBackend);

	// WARNING: The order of these operations is critical. All spaces must be
	// created before attaching the action set to the session, otherwise the
	// runtime will not recognize the spaces and they will not function
	// correctly.
	_handsMotionActions->createHandsMotionSpaces(deviceBackend);

	attachSessionActionSet(deviceBackend);
}

evan::XrManageActions::~XrManageActions()
{
	this->getLogger().info() << "Destroying XrManageActions";

	xrDestroyActionSet(_actionSet);
}

std::vector<std::shared_ptr<utility::event::Event>>
	evan::XrManageActions::pollActions(XrDeviceBackend &deviceBackend)
{
	this->getLogger().debug() << "Polling actions from OpenXR runtime";

	std::vector<std::shared_ptr<utility::event::Event>> events;

	const XrActiveActionSet activeActionSet { _actionSet, XR_NULL_PATH };
	XrActionsSyncInfo syncInfo { XR_TYPE_ACTIONS_SYNC_INFO };
	syncInfo.countActiveActionSets = 1;
	syncInfo.activeActionSets	   = &activeActionSet;

	XrResult result = xrSyncActions(deviceBackend._session, &syncInfo);
	if (result != XR_SUCCESS) {
		this->getLogger().error() << "Failed to sync actions: " << result;
		return events;
	}
	auto handEvents = _handsMotionActions->getEvents(deviceBackend);
	events.insert(events.end(), handEvents.begin(), handEvents.end());

	auto buttonEvents = _manageButtonsActions->getEvents(
		deviceBackend, _handsMotionActions->_leftHandEvent,
		_handsMotionActions->_rightHandEvent);
	events.insert(events.end(), buttonEvents.begin(), buttonEvents.end());

	auto thumbStickEvents = _manageThumbStickActions->getEvents(deviceBackend);
	events.insert(events.end(), thumbStickEvents.begin(),
				  thumbStickEvents.end());
	return events;
}

void evan::XrManageActions::createActionSet(XrDeviceBackend &deviceBackend)
{
	this->getLogger().info() << "Creating main action set for XrManageActions";

	XrActionSetCreateInfo actionSetInfo { XR_TYPE_ACTION_SET_CREATE_INFO };
	std::strncpy(actionSetInfo.actionSetName, "gameplay",
				 XR_MAX_ACTION_SET_NAME_SIZE);
	std::strncpy(actionSetInfo.localizedActionSetName, "Gameplay",
				 XR_MAX_LOCALIZED_ACTION_SET_NAME_SIZE);
	actionSetInfo.priority = 0;

	XrResult result = xrCreateActionSet(deviceBackend._XrInstance,
										&actionSetInfo, &_actionSet);
	if (result != XR_SUCCESS) {
		this->getLogger().error() << "Failed to create action set: " << result;
		return;
	}
	this->getLogger().info()
		<< "Successfully created main action set for XrManageActions";
}

void evan::XrManageActions::attachSessionActionSet(
	XrDeviceBackend &deviceBackend)
{
	this->getLogger().info() << "Attaching action set to OpenXR session";

	XrSessionActionSetsAttachInfo attachInfo {
		XR_TYPE_SESSION_ACTION_SETS_ATTACH_INFO
	};
	attachInfo.countActionSets = 1;
	attachInfo.actionSets	   = &_actionSet;

	XrResult result =
		xrAttachSessionActionSets(deviceBackend._session, &attachInfo);
	if (result != XR_SUCCESS) {
		this->getLogger().error() << "Failed to attach action sets: " << result;
		return;
	}
	this->getLogger().info()
		<< "Successfully attached action set to OpenXR session";
}

void evan::XrManageActions::bindActionSets(XrDeviceBackend &deviceBackend)
{
	this->getLogger().info()
		<< "Binding action sets to OpenXR interaction profile";

	std::vector<XrActionSuggestedBinding> bindings {
		{ _handsMotionActions->_handAimAction,
		  InteractionProfile::stringToPath(deviceBackend._XrInstance,
										   "/user/hand/left/input/aim/pose") },
		{ _handsMotionActions->_handAimAction,
		  InteractionProfile::stringToPath(deviceBackend._XrInstance,
										   "/user/hand/right/input/aim/pose") },

		{ _handsMotionActions->_handGripAction,
		  InteractionProfile::stringToPath(deviceBackend._XrInstance,
										   "/user/hand/left/input/grip/pose") },
		{ _handsMotionActions->_handGripAction,
		  InteractionProfile::stringToPath(
			  deviceBackend._XrInstance, "/user/hand/right/input/grip/pose") },

		{ _manageButtonsActions->_buttonAAction->getAction(),
		  InteractionProfile::stringToPath(deviceBackend._XrInstance,
										   "/user/hand/right/input/a/click") },
		{ _manageButtonsActions->_buttonBAction->getAction(),
		  InteractionProfile::stringToPath(deviceBackend._XrInstance,
										   "/user/hand/right/input/b/click") },
		{ _manageButtonsActions->_buttonXAction->getAction(),
		  InteractionProfile::stringToPath(deviceBackend._XrInstance,
										   "/user/hand/left/input/x/click") },
		{ _manageButtonsActions->_buttonYAction->getAction(),
		  InteractionProfile::stringToPath(deviceBackend._XrInstance,
										   "/user/hand/left/input/y/click") },

		{ _manageThumbStickActions->_leftHandThumbStickActions->getAction(),
		  InteractionProfile::stringToPath(
			  deviceBackend._XrInstance, "/user/hand/left/input/thumbstick") },
		{ _manageThumbStickActions->_rightHandThumbStickActions->getAction(),
		  InteractionProfile::stringToPath(
			  deviceBackend._XrInstance, "/user/hand/right/input/thumbstick") },

		// ERROR: When trying to bind the menu and system buttons, all the
		// actions stop working and no events are generated.
		// {_manageButtonsActions->_buttonMenuAction->getAction(),
		// InteractionProfile::stringToPath(deviceBackend._XrInstance,
		// "/user/hand/right/input/menu/click")},
		// {_manageButtonsActions->_buttonSystemAction->getAction(),
		// InteractionProfile::stringToPath(deviceBackend._XrInstance,
		// "/user/hand/right/input/system/click")},
	};

	XrInteractionProfileSuggestedBinding suggestedBindings {
		XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING
	};
	suggestedBindings.interactionProfile = InteractionProfile::stringToPath(
		deviceBackend._XrInstance,
		"/interaction_profiles/oculus/touch_controller");
	suggestedBindings.countSuggestedBindings =
		static_cast<uint32_t>(bindings.size());
	suggestedBindings.suggestedBindings = bindings.data();
	XrResult result						= xrSuggestInteractionProfileBindings(
		deviceBackend._XrInstance, &suggestedBindings);
	if (result != XR_SUCCESS) {
		this->getLogger().error()
			<< "Failed to suggest interaction profile bindings: " << result;
		return;
	}
	this->getLogger().info()
		<< "Successfully suggested interaction profile bindings";
}