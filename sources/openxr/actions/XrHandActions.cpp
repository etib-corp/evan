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

    // Create hand pose action
    XrActionCreateInfo handPoseActionInfo{XR_TYPE_ACTION_CREATE_INFO};
    std::strncpy(handPoseActionInfo.actionName, (_handPath + "/input/aim/pose").c_str(), XR_MAX_ACTION_NAME_SIZE);
    std::strncpy(handPoseActionInfo.localizedActionName, (_handPath + " Pose").c_str(), XR_MAX_LOCALIZED_ACTION_NAME_SIZE);
    handPoseActionInfo.actionType = XR_ACTION_TYPE_POSE_INPUT;
    handPoseActionInfo.countSubactionPaths = 0;
    handPoseActionInfo.subactionPaths = nullptr;

    XrResult result = xrCreateAction(actionSet, &handPoseActionInfo, &_handAimAction);
    if (result != XR_SUCCESS) {
        throw std::runtime_error("Failed to create hand pose action");
    }

    // Create hand grip action
    XrActionCreateInfo handGripActionInfo{XR_TYPE_ACTION_CREATE_INFO};
    std::strncpy(handGripActionInfo.actionName, (_handPath + "/input/grip/pose").c_str(), XR_MAX_ACTION_NAME_SIZE);
    std::strncpy(handGripActionInfo.localizedActionName, (_handPath + " Grip").c_str(), XR_MAX_LOCALIZED_ACTION_NAME_SIZE);
    handGripActionInfo.actionType = XR_ACTION_TYPE_FLOAT_INPUT;
    handGripActionInfo.countSubactionPaths = 0;
    handGripActionInfo.subactionPaths = nullptr;

    result = xrCreateAction(actionSet, &handGripActionInfo, &_handGripAction);
    if (result != XR_SUCCESS) {
        throw std::runtime_error("Failed to create hand grip action");
    }

    // Get action paths
    _handAimPath = evan::InteractionProfile::stringToPath(deviceBackend._XrInstance, (_handPath + "/aim").c_str());

    _handGripPath = evan::InteractionProfile::stringToPath(deviceBackend._XrInstance, (_handPath + "/grip").c_str());

    std::vector<XrActionSuggestedBinding> bindings = {
        {_handAimAction, _handAimPath},
        {_handGripAction, _handGripPath}
    };
    XrInteractionProfileSuggestedBinding suggestedBindings{XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING};
    suggestedBindings.interactionProfile = evan::InteractionProfile::stringToPath(deviceBackend._XrInstance, "/interaction_profiles/khr/simple_controller");
    suggestedBindings.suggestedBindings = bindings.data();
    suggestedBindings.countSuggestedBindings = static_cast<uint32_t>(bindings.size());
    result = xrSuggestInteractionProfileBindings(deviceBackend._XrInstance, &suggestedBindings);
    if (result != XR_SUCCESS) {
        throw std::runtime_error("Failed to suggest interaction profile bindings");
    }

    XrActionSpaceCreateInfo actionSpaceCreateInfo{XR_TYPE_ACTION_SPACE_CREATE_INFO};
    actionSpaceCreateInfo.action = _handAimAction;
    actionSpaceCreateInfo.poseInActionSpace.orientation.w = 1.0f; // Identity orientation
    actionSpaceCreateInfo.subactionPath = _handAimPath;
    result = xrCreateActionSpace(deviceBackend._session, &actionSpaceCreateInfo, &_handAimActionSpace);
    if (result != XR_SUCCESS) {
        throw std::runtime_error("Failed to create hand aim action space");
    }
}

evan::XrHandActions::~XrHandActions()
{
    xrDestroyAction(_handAimAction);
    xrDestroyAction(_handGripAction);
    xrDestroySpace(_handAimActionSpace);
}

std::vector<std::shared_ptr<utility::event::Event>> evan::XrHandActions::getEvents(evan::XrDeviceBackend &deviceBackend)
{
    std::vector<std::shared_ptr<utility::event::Event>> events;

    XrActionStateGetInfo getInfo{XR_TYPE_ACTION_STATE_GET_INFO};

    getInfo.action = _handAimAction;
    XrActionStatePose aimPose{XR_TYPE_ACTION_STATE_POSE};
    XrResult result = xrGetActionStatePose(deviceBackend._session, &getInfo, &aimPose);
    if (result == XR_SUCCESS && aimPose.isActive) {
        std::cout << "Hand aim pose is active" << std::endl;
        XrSpaceLocation handAimLocation{XR_TYPE_SPACE_LOCATION};
        xrLocateSpace(_handAimActionSpace, deviceBackend._space, deviceBackend._predictedDisplayTime, &handAimLocation);
        std::cout << "Hand aim position: (" << handAimLocation.pose.position.x << ", "
                  << handAimLocation.pose.position.y << ", "
                  << handAimLocation.pose.position.z << ")" << std::endl;
    }

    return events;
}