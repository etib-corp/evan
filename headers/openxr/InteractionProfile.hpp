/*
** EPITECH PROJECT, 2026
** evan
** File description:
** InteractionProfile
*/

#pragma once

#include "EvanPlatform.hpp"
#include <openxr/openxr.h>

#include <string>
#include <vector>

namespace evan {
    class InteractionProfile {
        public:
            InteractionProfile();
            virtual ~InteractionProfile() = default;

            static std::string getRuntimeName(XrInstance instance);
            static std::string getSystemName(XrInstance instance, XrSystemId systemId);

            static XrPath stringToPath(XrInstance instance, const std::string &pathString);
            static std::string pathToString(XrInstance instance, XrPath path);

            static std::string getCurrentInteractionProfilePath(
                XrInstance instance,
                XrSession session,
                const std::string &userPathString);

            static std::vector<XrPath>
                enumerateBoundSourcesForAction(XrInstance instance,
                    XrSession session,
                    XrAction action);
            static std::vector<std::string>
                enumerateBoundSourcePathStringsForAction(
                    XrInstance instance,
                    XrSession session,
                    XrAction action);

        private:
            XrActionSet _actionSet;
    };
} // namespace evan