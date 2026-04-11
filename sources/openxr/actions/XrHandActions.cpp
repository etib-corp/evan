/*
** ETIB PROJECT, 2026
** evan
** File description:
** XrHandActions
*/

#include "openxr/actions/XrHandActions.hpp"

#include "openxr/XrDeviceBackend.hpp"

evan::XrHandActions::XrHandActions(HandType handType, XrActionSet actionSet, XrDeviceBackend &deviceBackend)
{
    _handPath = (handType == HandType::Left) ? "/user/hand/left" : "/user/hand/right";
    _prefixActionName = (handType == HandType::Left) ? "left_hand" : "right_hand";

    // Create hand pose action
    XrActionCreateInfo handPoseActionInfo{XR_TYPE_ACTION_CREATE_INFO};
    std::strncpy(handPoseActionInfo.actionName, (_prefixActionName + "_pose").c_str(), XR_MAX_ACTION_NAME_SIZE);
    std::strncpy(handPoseActionInfo.localizedActionName, (_prefixActionName + "_pose").c_str(), XR_MAX_LOCALIZED_ACTION_NAME_SIZE);
    handPoseActionInfo.actionType = XR_ACTION_TYPE_POSE_INPUT;
    handPoseActionInfo.countSubactionPaths = 0;
    handPoseActionInfo.subactionPaths = nullptr;

    XrResult result = xrCreateAction(actionSet, &handPoseActionInfo, &_handAction);
    if (result != XR_SUCCESS) {
        throw std::runtime_error("Failed to create hand pose action");
    }



    XrActionSpaceCreateInfo actionSpaceCreateInfo{XR_TYPE_ACTION_SPACE_CREATE_INFO};
    actionSpaceCreateInfo.action = _handAction;
    actionSpaceCreateInfo.poseInActionSpace = { {0, 0, 0, 1}, {0, 0, 0} };

    result = xrCreateActionSpace(deviceBackend._session, &actionSpaceCreateInfo, &_handActionSpace);

    if (result != XR_SUCCESS) {
        throw std::runtime_error("Failed to create hand aim action space");
    }

    // Get action paths
    _handXrPath = evan::InteractionProfile::stringToPath(deviceBackend._XrInstance, (_handPath + "/input/grip/pose").c_str());

    std::vector<XrActionSuggestedBinding> bindings = {
        {_handAction, _handXrPath},
    };

    XrInteractionProfileSuggestedBinding suggestedBindings{XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING};
    suggestedBindings.interactionProfile = evan::InteractionProfile::stringToPath(deviceBackend._XrInstance, "/interaction_profiles/khr/simple_controller");
    suggestedBindings.suggestedBindings = bindings.data();
    suggestedBindings.countSuggestedBindings = static_cast<uint32_t>(bindings.size());
    result = xrSuggestInteractionProfileBindings(deviceBackend._XrInstance, &suggestedBindings);
    if (result != XR_SUCCESS) {
        throw std::runtime_error("Failed to suggest interaction profile bindings");
    }

    XrSessionActionSetsAttachInfo attachInfo{XR_TYPE_SESSION_ACTION_SETS_ATTACH_INFO};
    attachInfo.countActionSets = 1;
    attachInfo.actionSets = &actionSet;
    result = xrAttachSessionActionSets(deviceBackend._session, &attachInfo);
    if (result != XR_SUCCESS) {
        throw std::runtime_error("Failed to attach action sets to session");
    }
}

evan::XrHandActions::~XrHandActions()
{
    xrDestroyAction(_handAction);
    xrDestroyAction(_handGripAction);
    xrDestroySpace(_handActionSpace);
}

std::vector<std::shared_ptr<utility::event::Event>> evan::XrHandActions::getEvents(evan::XrDeviceBackend &deviceBackend)
{
    std::vector<std::shared_ptr<utility::event::Event>> events;

    XrActionStateGetInfo getInfo{XR_TYPE_ACTION_STATE_GET_INFO};

    getInfo.action = _handAction;

    XrActionStatePose poseState{XR_TYPE_ACTION_STATE_POSE};
    XrResult result = xrGetActionStatePose(deviceBackend._session, &getInfo, &poseState);
    if (result != XR_SUCCESS) {
        throw std::runtime_error("Failed to get hand pose state");
    }

    XrSpaceLocation handLocation{XR_TYPE_SPACE_LOCATION};

    xrLocateSpace(_handActionSpace, deviceBackend._space, deviceBackend._predictedDisplayTime, &handLocation);

    if ((poseState.isActive) && (handLocation.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT) &&
        (handLocation.locationFlags & XR_SPACE_LOCATION_ORIENTATION_VALID_BIT)) {
        std::cout << "Hand pose: position = (" << handLocation.pose.position.x << ", " << handLocation.pose.position.y << ", "
                  << handLocation.pose.position.z << "), orientation = (" << handLocation.pose.orientation.x << ", "
                  << handLocation.pose.orientation.y << ", " << handLocation.pose.orientation.z << ", "
                  << handLocation.pose.orientation.w << ")" << std::endl;
    }

    return events;
}