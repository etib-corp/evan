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

            std::string getRuntimeName(XrInstance instance) const;
            std::string getSystemName(XrInstance instance, XrSystemId systemId) const;

            XrPath stringToPath(XrInstance instance, const std::string &pathString) const;
            std::string pathToString(XrInstance instance, XrPath path) const;

            std::string getCurrentInteractionProfilePath(
                XrInstance instance,
                XrSession session,
                const std::string &userPathString) const;

            std::vector<XrPath>
                enumerateBoundSourcesForAction(XrInstance instance,
                    XrSession session,
                    XrAction action) const;
            std::vector<std::string>
                enumerateBoundSourcePathStringsForAction(
                    XrInstance instance,
                    XrSession session,
                    XrAction action) const;

        private:
    };
} // namespace evan