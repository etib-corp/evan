/*
** ETIB PROJECT, 2026
** evan
** File description:
** XrManageActions
*/

#include "openxr/XrManageActions.hpp"
#include "openxr/XrDeviceBackend.hpp"

evan::XrManageActions::XrManageActions(XrDeviceBackend &deviceBackend)
{
    createActionSet(deviceBackend);

    _handsMotionActions = std::make_unique<XrHandsMotionActions>(_actionSet, deviceBackend);
    _manageButtonsActions = std::make_unique<XrManageButtonsActions>(_actionSet, deviceBackend);

    bindActionSets(deviceBackend);

    // WARNING: The order of these operations is critical. All spaces must be created before attaching the action set to the session,
    // otherwise the runtime will not recognize the spaces and they will not function correctly.
    _handsMotionActions->createHandsMotionSpaces(deviceBackend);

    attachSessionActionSet(deviceBackend);
}

evan::XrManageActions::~XrManageActions()
{
    xrDestroyActionSet(_actionSet);
}

std::vector<std::unique_ptr<utility::event::Event>> evan::XrManageActions::pollActions(XrDeviceBackend &deviceBackend)
{
    std::vector<std::unique_ptr<utility::event::Event>> events;

    const XrActiveActionSet activeActionSet{_actionSet, XR_NULL_PATH};
    XrActionsSyncInfo syncInfo{XR_TYPE_ACTIONS_SYNC_INFO};
    syncInfo.countActiveActionSets = 1;
    syncInfo.activeActionSets = &activeActionSet;

    XrResult result = xrSyncActions(deviceBackend._session, &syncInfo);
    if (result != XR_SUCCESS) {
        std::cerr << "Failed to sync actions: " << result << std::endl;
        return events;
    }
    auto handEvents = _handsMotionActions->getEvents(deviceBackend);
    events.insert(events.end(), std::make_move_iterator(handEvents.begin()), std::make_move_iterator(handEvents.end()));
    auto buttonEvents = _manageButtonsActions->getEvents(deviceBackend);
    events.insert(events.end(), std::make_move_iterator(buttonEvents.begin()), std::make_move_iterator(buttonEvents.end()));
    return events;
}

void evan::XrManageActions::createActionSet(XrDeviceBackend &deviceBackend)
{
    XrActionSetCreateInfo actionSetInfo{XR_TYPE_ACTION_SET_CREATE_INFO};
    std::strncpy(actionSetInfo.actionSetName, "gameplay", XR_MAX_ACTION_SET_NAME_SIZE);
    std::strncpy(actionSetInfo.localizedActionSetName, "Gameplay", XR_MAX_LOCALIZED_ACTION_SET_NAME_SIZE);
    actionSetInfo.priority = 0;

    XrResult result = xrCreateActionSet(deviceBackend._XrInstance, &actionSetInfo, &_actionSet);
    if (result != XR_SUCCESS) {
        throw std::runtime_error("Failed to create action set");
    }
}

void evan::XrManageActions::attachSessionActionSet(XrDeviceBackend &deviceBackend)
{
    XrSessionActionSetsAttachInfo attachInfo{XR_TYPE_SESSION_ACTION_SETS_ATTACH_INFO};
    attachInfo.countActionSets = 1;
    attachInfo.actionSets = &_actionSet;

    XrResult result = xrAttachSessionActionSets(deviceBackend._session, &attachInfo);
    if (result != XR_SUCCESS) {
        std::cerr << "Failed to attach action sets: " << result << std::endl;
        throw std::runtime_error("Failed to attach action sets");
    }
}

void evan::XrManageActions::bindActionSets(XrDeviceBackend &deviceBackend)
{
    std::vector<XrActionSuggestedBinding> bindings {
        {_handsMotionActions->_handAimAction, InteractionProfile::stringToPath(deviceBackend._XrInstance, "/user/hand/left/input/aim/pose")},
        {_handsMotionActions->_handAimAction, InteractionProfile::stringToPath(deviceBackend._XrInstance, "/user/hand/right/input/aim/pose")},

        {_handsMotionActions->_handGripAction, InteractionProfile::stringToPath(deviceBackend._XrInstance, "/user/hand/left/input/grip/pose")},
        {_handsMotionActions->_handGripAction, InteractionProfile::stringToPath(deviceBackend._XrInstance, "/user/hand/right/input/grip/pose")},

        {_manageButtonsActions->_buttonAAction->getAction(), InteractionProfile::stringToPath(deviceBackend._XrInstance, "/user/hand/right/input/a/click")},
    };

    XrInteractionProfileSuggestedBinding suggestedBindings{XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING};
    suggestedBindings.interactionProfile = InteractionProfile::stringToPath(deviceBackend._XrInstance, "/interaction_profiles/oculus/touch_controller");
    suggestedBindings.countSuggestedBindings = static_cast<uint32_t>(bindings.size());
    suggestedBindings.suggestedBindings = bindings.data();
    XrResult result = xrSuggestInteractionProfileBindings(deviceBackend._XrInstance, &suggestedBindings);
    if (result != XR_SUCCESS) {
        std::cerr << "Failed to suggest interaction profile bindings: " << result << std::endl;
        return;
    }
}