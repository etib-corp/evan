/*
** ETIB PROJECT, 2026
** evan
** File description:
** IXrAction
*/

#include "openxr/actions/AXrAction.hpp"

////////////////////
// Public Methods //
////////////////////

XrAction evan::AXrAction::getAction() const
{
    return _action;
}

const std::string &evan::AXrAction::getActionName() const
{
    return _actionName;
}

const std::string &evan::AXrAction::getBindingPath() const
{
    return _bindingPath;
}

void evan::AXrAction::createAction(const PropertiesXrActions &properties)
{
    _actionName = properties.actionName;
    _bindingPath = properties.bindingPath;

    XrActionCreateInfo actionCreateInfo{XR_TYPE_ACTION_CREATE_INFO};
    actionCreateInfo.actionType = properties.actionType;
    actionCreateInfo.countSubactionPaths = 0;
    actionCreateInfo.subactionPaths = nullptr;
    strcpy(actionCreateInfo.localizedActionName, properties.actionName.c_str());
    strcpy(actionCreateInfo.actionName, properties.actionName.c_str());

    XrResult result = xrCreateAction(properties.actionSet, &actionCreateInfo, &_action);
    if (result != XR_SUCCESS) {
        throw std::runtime_error("Failed to create action: " + std::to_string(result));
    }
}