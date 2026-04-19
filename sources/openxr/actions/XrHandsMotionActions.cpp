/*
** ETIB PROJECT, 2026
** evan
** File description:
** XrHandsMotionActions
*/

#include "openxr/actions/XrHandsMotionActions.hpp"

#include "openxr/XrDeviceBackend.hpp"

evan::XrHandsMotionActions::XrHandsMotionActions(XrActionSet actionSet, XrDeviceBackend &deviceBackend)
{
    createHandsMotionActions(actionSet, deviceBackend);
}

evan::XrHandsMotionActions::~XrHandsMotionActions()
{
    xrDestroyAction(_handAimAction);
    xrDestroySpace(_handAimSpace[0]);
    xrDestroySpace(_handAimSpace[1]);

    xrDestroyAction(_handGripAction);
    xrDestroySpace(_handGripSpace[0]);
    xrDestroySpace(_handGripSpace[1]);
}

////////////////////
// Public Methods //
////////////////////

std::vector<std::unique_ptr<utility::event::Event>> evan::XrHandsMotionActions::getEvents(evan::XrDeviceBackend &deviceBackend)
{
    std::vector<std::unique_ptr<utility::event::Event>> events;

    for (int i = 0; i < 2; i++) {

        auto handEvent = std::make_unique<utility::event::HandMotionEvent>();
        handEvent->setHandType(i == 0 ? utility::event::HandEvent::HandType::Left : utility::event::HandEvent::HandType::Right);

        XrActionStateGetInfo getInfo{XR_TYPE_ACTION_STATE_GET_INFO};
        getInfo.action = _handAimAction;
        getInfo.subactionPath = deviceBackend._handActionSubactionPath[i];

        XrActionStatePose poseState{XR_TYPE_ACTION_STATE_POSE};
        xrGetActionStatePose(deviceBackend._session, &getInfo, &poseState);

        if (!poseState.isActive) {
            std::cout << (i == 0 ? "LEFT" : "RIGHT") << " hand is not active.\n";
            continue;
        }

        XrSpaceLocation aimLocation{XR_TYPE_SPACE_LOCATION};

        XrResult res = xrLocateSpace(
            _handAimSpace[i],
            deviceBackend._space,
            deviceBackend._predictedDisplayTime,
            &aimLocation
        );

        if (XR_FAILED(res)) {
            std::cerr << "Failed to locate hand space: " << res << std::endl;
            continue;
        }

        if ((aimLocation.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT) &&
            (aimLocation.locationFlags & XR_SPACE_LOCATION_ORIENTATION_VALID_BIT)) {

            utility::graphic::PoseF pose;
            pose.setPosition(utility::graphic::Position<float>(aimLocation.pose.position.x, aimLocation.pose.position.y, aimLocation.pose.position.z));
            pose.setOrientation(utility::graphic::Orientation<float>(aimLocation.pose.orientation.x, aimLocation.pose.orientation.y, aimLocation.pose.orientation.z, aimLocation.pose.orientation.w));

            handEvent->setAim(pose);

            std::cout << (i == 0 ? "LEFT" : "RIGHT") << " HAND AIM\n";
            std::cout << "Position: " << aimLocation.pose.position.x << ", " << aimLocation.pose.position.y << ", " << aimLocation.pose.position.z << "\n";
            std::cout << "Orientation: " << aimLocation.pose.orientation.x << ", " << aimLocation.pose.orientation.y << ", " << aimLocation.pose.orientation.z << ", " << aimLocation.pose.orientation.w << "\n";
        }

        XrSpaceLocation gripLocation{XR_TYPE_SPACE_LOCATION};

        res = xrLocateSpace(
            _handGripSpace[i],
            deviceBackend._space,
            deviceBackend._predictedDisplayTime,
            &gripLocation
        );

        if (XR_FAILED(res)) {
            std::cerr << "Failed to locate hand grip space: " << res << std::endl;
            continue;
        }

        if ((gripLocation.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT) &&
            (gripLocation.locationFlags & XR_SPACE_LOCATION_ORIENTATION_VALID_BIT)) {

            utility::graphic::PoseF grip;
            grip.setPosition(utility::graphic::Position<float>(gripLocation.pose.position.x, gripLocation.pose.position.y, gripLocation.pose.position.z));
            grip.setOrientation(utility::graphic::Orientation<float>(gripLocation.pose.orientation.x, gripLocation.pose.orientation.y, gripLocation.pose.orientation.z, gripLocation.pose.orientation.w));

            handEvent->setGrip(grip);

            std::cout << (i == 0 ? "LEFT" : "RIGHT") << " HAND GRIP\n";
            std::cout << "Position: " << gripLocation.pose.position.x << ", " << gripLocation.pose.position.y << ", " << gripLocation.pose.position.z << "\n";
            std::cout << "Orientation: " << gripLocation.pose.orientation.x << ", " << gripLocation.pose.orientation.y << ", " << gripLocation.pose.orientation.z << ", " << gripLocation.pose.orientation.w << "\n";
        }


        events.push_back(std::move(handEvent));
    }
    return events;
}

void evan::XrHandsMotionActions::createHandsMotionSpaces(XrDeviceBackend &deviceBackend)
{
    for (int i = 0; i < 2; i++) {
        XrActionSpaceCreateInfo spaceCreateInfo{XR_TYPE_ACTION_SPACE_CREATE_INFO};
        spaceCreateInfo.action = _handAimAction;
        spaceCreateInfo.subactionPath = deviceBackend._handActionSubactionPath[i];
        spaceCreateInfo.poseInActionSpace.orientation.w = 1.0f;

        XrResult result = xrCreateActionSpace(deviceBackend._session, &spaceCreateInfo, &_handAimSpace[i]);
        if (result != XR_SUCCESS) {
            std::cerr << "Failed to create hand space: " << result << std::endl;
            throw std::runtime_error("Failed to create hand space");
        }

        spaceCreateInfo.action = _handGripAction;
        result = xrCreateActionSpace(deviceBackend._session, &spaceCreateInfo, &_handGripSpace[i]);
        if (result != XR_SUCCESS) {
            std::cerr << "Failed to create hand grip space: " << result << std::endl;
            throw std::runtime_error("Failed to create hand grip space");
        }
    }
}

/////////////////////
// Private Methods //
/////////////////////

void evan::XrHandsMotionActions::createHandsMotionActions(XrActionSet actionSet, XrDeviceBackend &deviceBackend)
{
    // Create pose action
    XrActionCreateInfo actionInfo{XR_TYPE_ACTION_CREATE_INFO};
    actionInfo.actionType = XR_ACTION_TYPE_POSE_INPUT;
    actionInfo.countSubactionPaths = 2;
    actionInfo.subactionPaths = deviceBackend._handActionSubactionPath.data();

    strcpy(actionInfo.actionName, "hand_pose");
    strcpy(actionInfo.localizedActionName, "Hand Pose");

    XrResult result = xrCreateAction(actionSet, &actionInfo, &_handAimAction);
    if (result != XR_SUCCESS) {
        std::cerr << "Failed to create hand pose action: " << result << std::endl;
        throw std::runtime_error("Failed to create hand pose action");
    }

    // Create grip action
    actionInfo.actionType = XR_ACTION_TYPE_POSE_INPUT;
    actionInfo.countSubactionPaths = 2;
    actionInfo.subactionPaths = deviceBackend._handActionSubactionPath.data();

    strcpy(actionInfo.actionName, "hand_grip");
    strcpy(actionInfo.localizedActionName, "Hand Grip");

    result = xrCreateAction(actionSet, &actionInfo, &_handGripAction);
    if (result != XR_SUCCESS) {
        std::cerr << "Failed to create hand grip action: " << result << std::endl;
        throw std::runtime_error("Failed to create hand grip action");
    }
}
