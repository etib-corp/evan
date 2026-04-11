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

    _leftHandActions = std::make_unique<XrHandActions>(XrHandActions::HandType::Left, _actionSet, deviceBackend);
    _rightHandActions = std::make_unique<XrHandActions>(XrHandActions::HandType::Right, _actionSet, deviceBackend);
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
    const auto leftEvents = _leftHandActions->getEvents(deviceBackend);
    events.insert(events.end(), leftEvents.begin(), leftEvents.end());
    const auto rightEvents = _rightHandActions->getEvents(deviceBackend);
    events.insert(events.end(), rightEvents.begin(), rightEvents.end());
    return events;
}