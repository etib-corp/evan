/*
** ETIB PROJECT, 2026
** evan
** File description:
** XrHandsMotionActions
*/

#include "evan/openxr/actions/XrHandsMotionActions.hpp"

#include "evan/openxr/XrDeviceBackend.hpp"

evan::XrHandsMotionActions::XrHandsMotionActions(XrActionSet actionSet,
												 XrDeviceBackend &deviceBackend)
{
	createHandsMotionActions(actionSet, deviceBackend);
}

evan::XrHandsMotionActions::~XrHandsMotionActions()
{
	this->getLogger().info() << "Destroying hand motion actions and spaces...";

	xrDestroyAction(_handAimAction);
	xrDestroySpace(_handAimSpace[0]);
	xrDestroySpace(_handAimSpace[1]);
	this->getLogger().info() << "Hand motion actions and spaces destroyed.";

	xrDestroyAction(_handGripAction);
	xrDestroySpace(_handGripSpace[0]);
	xrDestroySpace(_handGripSpace[1]);
	this->getLogger().info() << "Hand grip actions and spaces destroyed.";
}

////////////////////
// Public Methods //
////////////////////

std::vector<std::shared_ptr<utility::event::Event>>
	evan::XrHandsMotionActions::getEvents(evan::XrDeviceBackend &deviceBackend)
{
	std::vector<std::shared_ptr<utility::event::Event>> events;

	_leftHandEvent = createHandMotionEvent(deviceBackend, 0);
	if (_leftHandEvent) {
		events.push_back(_leftHandEvent);
	}
	_rightHandEvent = createHandMotionEvent(deviceBackend, 1);
	if (_rightHandEvent) {
		events.push_back(_rightHandEvent);
	}
	return events;
}

void evan::XrHandsMotionActions::createHandsMotionSpaces(
	XrDeviceBackend &deviceBackend)
{
	this->getLogger().info() << "Creating hand motion spaces...";

	for (int i = 0; i < 2; i++) {
		this->getLogger().info()
			<< "Creating spaces for " << std::string(i == 0 ? "LEFT" : "RIGHT")
			<< " hand...";
		XrActionSpaceCreateInfo spaceCreateInfo {
			XR_TYPE_ACTION_SPACE_CREATE_INFO
		};
		spaceCreateInfo.action = _handAimAction;
		spaceCreateInfo.subactionPath =
			deviceBackend._handActionSubactionPath[i];
		spaceCreateInfo.poseInActionSpace.orientation.w = 1.0f;

		XrResult result = xrCreateActionSpace(
			deviceBackend._session, &spaceCreateInfo, &_handAimSpace[i]);
		if (result != XR_SUCCESS) {
			this->getLogger().error()
				<< "Failed to create hand aim space: " << result;
			return;
		}
		this->getLogger().info() << "Hand aim space created successfully.";

		spaceCreateInfo.action = _handGripAction;
		result = xrCreateActionSpace(deviceBackend._session, &spaceCreateInfo,
									 &_handGripSpace[i]);
		if (result != XR_SUCCESS) {
			this->getLogger().error()
				<< "Failed to create hand grip space: " << result;
			return;
		}
		this->getLogger().info() << "Hand grip space created successfully.";
	}
	this->getLogger().info() << "Hand motion spaces created successfully.";
}

/////////////////////
// Private Methods //
/////////////////////

void evan::XrHandsMotionActions::createHandsMotionActions(
	XrActionSet actionSet, XrDeviceBackend &deviceBackend)
{
	this->getLogger().info() << "Creating hand motion actions...";

	// Create pose action
	XrActionCreateInfo actionInfo { XR_TYPE_ACTION_CREATE_INFO };
	actionInfo.actionType		   = XR_ACTION_TYPE_POSE_INPUT;
	actionInfo.countSubactionPaths = 2;
	actionInfo.subactionPaths = deviceBackend._handActionSubactionPath.data();

	strcpy(actionInfo.actionName, "hand_pose");
	strcpy(actionInfo.localizedActionName, "Hand Pose");

	XrResult result = xrCreateAction(actionSet, &actionInfo, &_handAimAction);
	if (result != XR_SUCCESS) {
		this->getLogger().error()
			<< "Failed to create hand pose action: " << result;
		return;
	}

	this->getLogger().info() << "Hand pose action created successfully.";

	// Create grip action
	actionInfo.actionType		   = XR_ACTION_TYPE_POSE_INPUT;
	actionInfo.countSubactionPaths = 2;
	actionInfo.subactionPaths = deviceBackend._handActionSubactionPath.data();

	strcpy(actionInfo.actionName, "hand_grip");
	strcpy(actionInfo.localizedActionName, "Hand Grip");

	result = xrCreateAction(actionSet, &actionInfo, &_handGripAction);
	if (result != XR_SUCCESS) {
		this->getLogger().error()
			<< "Failed to create hand grip action: " << result;
		return;
	}
	this->getLogger().info() << "Hand grip action created successfully.";
}

std::shared_ptr<utility::event::HandMotionEvent>
	evan::XrHandsMotionActions::createHandMotionEvent(
		XrDeviceBackend &deviceBackend, int handIndex)
{
	auto handEvent = std::make_shared<utility::event::HandMotionEvent>();
	handEvent->setHandType(handIndex == 0
							   ? utility::event::HandEvent::HandType::Left
							   : utility::event::HandEvent::HandType::Right);
	XrActionStateGetInfo getInfo { XR_TYPE_ACTION_STATE_GET_INFO };
	getInfo.action		  = _handAimAction;
	getInfo.subactionPath = deviceBackend._handActionSubactionPath[handIndex];

	XrActionStatePose poseState { XR_TYPE_ACTION_STATE_POSE };
	xrGetActionStatePose(deviceBackend._session, &getInfo, &poseState);

	if (!poseState.isActive) {
		this->getLogger().info()
			<< (handIndex == 0 ? std::string("LEFT") : std::string("RIGHT"))
			<< std::string(" hand pose action is not active.");
		return nullptr;
	}

	XrSpaceLocation aimLocation { XR_TYPE_SPACE_LOCATION };

	XrResult res =
		xrLocateSpace(_handAimSpace[handIndex], deviceBackend._space,
					  deviceBackend._predictedDisplayTime, &aimLocation);

	if (XR_FAILED(res)) {
		this->getLogger().error() << "Failed to locate hand space: " << res;
		return nullptr;
	}

	if ((aimLocation.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT)
		&& (aimLocation.locationFlags
			& XR_SPACE_LOCATION_ORIENTATION_VALID_BIT)) {
		utility::graphic::PoseF pose;
		pose.setPosition(utility::graphic::Position<float>(
			aimLocation.pose.position.x, aimLocation.pose.position.y,
			aimLocation.pose.position.z));
		pose.setOrientation(utility::graphic::Orientation<float>(
			aimLocation.pose.orientation.x, aimLocation.pose.orientation.y,
			aimLocation.pose.orientation.z, aimLocation.pose.orientation.w));

		handEvent->setAim(pose);

		this->getLogger().info()
			<< (handIndex == 0 ? std::string("LEFT") : std::string("RIGHT"))
			<< std::string(" HAND AIM");
		this->getLogger().info() << "Position: " << aimLocation.pose.position.x
								 << ", " << aimLocation.pose.position.y << ", "
								 << aimLocation.pose.position.z;
		this->getLogger().info()
			<< "Orientation: " << aimLocation.pose.orientation.x << ", "
			<< aimLocation.pose.orientation.y << ", "
			<< aimLocation.pose.orientation.z << ", "
			<< aimLocation.pose.orientation.w;
	}

	XrSpaceLocation gripLocation { XR_TYPE_SPACE_LOCATION };

	res = xrLocateSpace(_handGripSpace[handIndex], deviceBackend._space,
						deviceBackend._predictedDisplayTime, &gripLocation);

	if (XR_FAILED(res)) {
		this->getLogger().error()
			<< "Failed to locate hand grip space: " << res;
		return nullptr;
	}

	if ((gripLocation.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT)
		&& (gripLocation.locationFlags
			& XR_SPACE_LOCATION_ORIENTATION_VALID_BIT)) {
		utility::graphic::PoseF grip;
		grip.setPosition(utility::graphic::Position<float>(
			gripLocation.pose.position.x, gripLocation.pose.position.y,
			gripLocation.pose.position.z));
		grip.setOrientation(utility::graphic::Orientation<float>(
			gripLocation.pose.orientation.x, gripLocation.pose.orientation.y,
			gripLocation.pose.orientation.z, gripLocation.pose.orientation.w));

		handEvent->setGrip(grip);

		this->getLogger().info()
			<< (handIndex == 0 ? std::string("LEFT") : std::string("RIGHT"))
			<< std::string(" HAND GRIP");
		this->getLogger().info() << "Position: " << gripLocation.pose.position.x
								 << ", " << gripLocation.pose.position.y << ", "
								 << gripLocation.pose.position.z;
		this->getLogger().info()
			<< "Orientation: " << gripLocation.pose.orientation.x << ", "
			<< gripLocation.pose.orientation.y << ", "
			<< gripLocation.pose.orientation.z << ", "
			<< gripLocation.pose.orientation.w;
		return handEvent;
	}
	return nullptr;
}