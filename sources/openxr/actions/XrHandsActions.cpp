/*
** ETIB PROJECT, 2026
** evan
** File description:
** XrHandsActions
*/

#include "openxr/actions/XrHandsActions.hpp"

#include "openxr/XrDeviceBackend.hpp"

evan::XrHandsActions::XrHandsActions(XrActionSet actionSet, XrDeviceBackend &deviceBackend)
{
    createHandActions(actionSet, deviceBackend);

    XrActionSuggestedBinding bindings[4];
    bindings[0].action = _handAction;
    bindings[0].binding = evan::InteractionProfile::stringToPath(deviceBackend._XrInstance, "/user/hand/left/aim");
    bindings[1].action = _handAction;
    bindings[1].binding = evan::InteractionProfile::stringToPath(deviceBackend._XrInstance, "/user/hand/right/aim");
    bindings[2].action = _handGripAction;
    bindings[2].binding = evan::InteractionProfile::stringToPath(deviceBackend._XrInstance, "/user/hand/left/grip");
    bindings[3].action = _handGripAction;
    bindings[3].binding = evan::InteractionProfile::stringToPath(deviceBackend._XrInstance, "/user/hand/right/grip");

    XrInteractionProfileSuggestedBinding suggestedBindings{XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING};
    suggestedBindings.interactionProfile = evan::InteractionProfile::stringToPath(deviceBackend._XrInstance, "/interaction_profiles/khr/simple_controller");
    suggestedBindings.countSuggestedBindings = 4;
    suggestedBindings.suggestedBindings = bindings;
    xrSuggestInteractionProfileBindings(deviceBackend._XrInstance, &suggestedBindings);
}

evan::XrHandsActions::~XrHandsActions()
{
    xrDestroyAction(_handAction);
    xrDestroySpace(_handSpace[0]);
    xrDestroySpace(_handSpace[1]);

    xrDestroyAction(_handGripAction);
    xrDestroySpace(_handGripSpace);
}

void evan::XrHandsActions::createHandActions(XrActionSet actionSet, XrDeviceBackend &deviceBackend)
{
    // Convert paths
    xrStringToPath(deviceBackend._XrInstance, "/user/hand/left", &_handSubactionPath[0]);
    xrStringToPath(deviceBackend._XrInstance, "/user/hand/right", &_handSubactionPath[1]);

    // Create pose action
    XrActionCreateInfo actionInfo{XR_TYPE_ACTION_CREATE_INFO};
    actionInfo.actionType = XR_ACTION_TYPE_POSE_INPUT;
    actionInfo.countSubactionPaths = 2;
    actionInfo.subactionPaths = _handSubactionPath;

    strcpy(actionInfo.actionName, "hand_pose");
    strcpy(actionInfo.localizedActionName, "Hand Pose");

    xrCreateAction(actionSet, &actionInfo, &_handAction);

    // Create space per hand
    XrActionSpaceCreateInfo spaceInfo{XR_TYPE_ACTION_SPACE_CREATE_INFO};
    spaceInfo.action = _handAction;
    spaceInfo.poseInActionSpace.orientation.w = 1.0f;

    for (int i = 0; i < 2; i++) {
        spaceInfo.subactionPath = _handSubactionPath[i];
        xrCreateActionSpace(deviceBackend._session, &spaceInfo, &_handSpace[i]);
    }
}

std::vector<std::shared_ptr<utility::event::Event>> evan::XrHandsActions::getEvents(evan::XrDeviceBackend &deviceBackend)
{
    std::vector<std::shared_ptr<utility::event::Event>> events;

    for (int i = 0; i < 2; i++) {
        // 1. Check if active
        XrActionStateGetInfo getInfo{XR_TYPE_ACTION_STATE_GET_INFO};
        getInfo.action = _handAction;
        getInfo.subactionPath = _handSubactionPath[i];

        XrActionStatePose poseState{XR_TYPE_ACTION_STATE_POSE};
        xrGetActionStatePose(deviceBackend._session, &getInfo, &poseState);

        if (!poseState.isActive)
            continue;

        // 2. Get real pose
        XrSpaceLocation location{XR_TYPE_SPACE_LOCATION};

        XrResult res = xrLocateSpace(
            _handSpace[i],
            deviceBackend._space,
            deviceBackend._predictedDisplayTime,
            &location
        );

        if (XR_FAILED(res))
            continue;

        if ((location.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT) &&
            (location.locationFlags & XR_SPACE_LOCATION_ORIENTATION_VALID_BIT)) {


            std::cout << (i == 0 ? "LEFT" : "RIGHT") << " HAND\n";
            std::cout << "Position: " << location.pose.position.x << ", " << location.pose.position.y << ", " << location.pose.position.z << "\n";
            std::cout << "Orientation: " << location.pose.orientation.x << ", " << location.pose.orientation.y << ", " << location.pose.orientation.z << ", " << location.pose.orientation.w << "\n";
        }
    }
    return events;
}