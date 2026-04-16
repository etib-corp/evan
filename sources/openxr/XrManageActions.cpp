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
    XrActionSetCreateInfo actionSetInfo{XR_TYPE_ACTION_SET_CREATE_INFO};
    std::strncpy(actionSetInfo.actionSetName, "gameplay", XR_MAX_ACTION_SET_NAME_SIZE);
    std::strncpy(actionSetInfo.localizedActionSetName, "Gameplay", XR_MAX_LOCALIZED_ACTION_SET_NAME_SIZE);
    actionSetInfo.priority = 0;

    XrResult result = xrCreateActionSet(deviceBackend._XrInstance, &actionSetInfo, &_actionSet);
    if (result != XR_SUCCESS) {
        throw std::runtime_error("Failed to create action set");
    }

    _handsActions = std::make_unique<XrHandsActions>(_actionSet, deviceBackend);

    XrSessionActionSetsAttachInfo attachInfo{XR_TYPE_SESSION_ACTION_SETS_ATTACH_INFO};
    attachInfo.countActionSets = 1;
    attachInfo.actionSets = &_actionSet;

    result = xrAttachSessionActionSets(deviceBackend._session, &attachInfo);
    if (result != XR_SUCCESS) {
        std::cerr << "Failed to attach action sets: " << result << std::endl;
        throw std::runtime_error("Failed to attach action sets");
    }
    std::cout << "Polling actions for action set: " << _actionSet << std::endl;
}

evan::XrManageActions::~XrManageActions()
{
    xrDestroyActionSet(_actionSet);
}

std::vector<std::shared_ptr<utility::event::Event>> evan::XrManageActions::pollActions(XrDeviceBackend &deviceBackend)
{
    std::vector<std::shared_ptr<utility::event::Event>> events;

    const XrActiveActionSet activeActionSet{_actionSet, XR_NULL_PATH};
    XrActionsSyncInfo syncInfo{XR_TYPE_ACTIONS_SYNC_INFO};
    syncInfo.countActiveActionSets = 1;
    syncInfo.activeActionSets = &activeActionSet;

    XrResult result = xrSyncActions(deviceBackend._session, &syncInfo);
    if (result != XR_SUCCESS) {
        throw std::runtime_error("Failed to sync actions");
    }
    const auto handEvents = _handsActions->getEvents(deviceBackend);
    events.insert(events.end(), handEvents.begin(), handEvents.end());
    return events;
}