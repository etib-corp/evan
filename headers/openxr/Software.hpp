/*
** ETIB PROJECT, 2025
** evan
** File description:
** Software
*/

#pragma once

#include "ASoftware.hpp"
#include "openxr/AndroidPlatform.hpp"	 // Always include for PlatformData definition
#include "openxr/GraphicalContext.hpp"
#include "EvanPlatform.hpp"

#include <algorithm>
#include <cstring>
#include <iostream>
#include <map>
#include <memory>

namespace evan
{
	namespace openxr
	{
		/**
		 * @class Software
		 *
		 * @brief A class that represents a software implementation for an
		 * OpenXR platform.
		 *
		 * Inherits from the ASoftware abstract base class and provides
		 * functionality to create an OpenXR instance, initialize the system,
		 * and manage the platform and graphical context.
		 */
		class Software: public ASoftware
		{
			public:
			/**
			 * @brief Constructor for the Software class.
			 *
			 * Initializes the Software instance with the provided platform
			 * data. Creates an AndroidPlatform instance using the given
			 * platform data and sets it as the platform. The graphical context
			 * is initialized to nullptr.
			 *
			 * @param platformData A shared pointer to the PlatformData object
			 * containing platform-specific information.
			 */
			Software(const std::shared_ptr<PlatformData> &platformData);

			// Destructor
			~Software();

			protected:
			/**
			 * @brief Creates an OpenXR instance, initializes the graphical
			 * context, and sets up an OpenXR session.
			 *
			 * This function performs the following steps:
			 * 1. Checks if the OpenXR instance is already created. If so, it
			 * returns immediately.
			 * 2. Prepares a list of required extensions, including
			 * platform-specific and graphical context extensions.
			 * 3. Fills out the XrInstanceCreateInfo structure and creates the
			 * OpenXR instance.
			 * 4. Retrieves the OpenXR system ID for the head-mounted display
			 * form factor.
			 * 5. Initializes the graphical context and retrieves the Vulkan
			 * context.
			 * 6. Sets up the Vulkan graphics binding for OpenXR.
			 * 7. Creates an OpenXR session using the initialized OpenXR
			 * instance and system ID.
			 *
			 * @note If any step fails, an error message is logged to std::cerr,
			 * and the function returns without completing the remaining steps.
			 *
			 */
			void createInstance();

			/**
			 * @brief Initializes the OpenXR system by retrieving the system ID
			 * for the head-mounted display form factor.
			 *
			 * This function checks if the OpenXR instance is valid and then
			 * calls `xrGetSystem` to obtain the system ID for the
			 * `XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY` form factor. If successful,
			 * it stores the system ID in the `_XRsystemID` member variable.
			 *
			 * @note If the OpenXR instance is not valid or if `xrGetSystem`
			 * fails, an error message is logged to std::cerr.
			 */
			void initializeSystem();

			XrInstance _XRinstance =
				XR_NULL_HANDLE;	   // OpenXR instance handle
			XrSystemId _XRsystemID = XR_NULL_SYSTEM_ID;	   // OpenXR system ID
#ifdef __ANDROID__
			std::shared_ptr<AndroidPlatform>
				_platform;	  // Platform-specific implementation (e.g.,
							  // Android)
#endif
			private:
		};

	}	 // namespace openxr
}	 // namespace evan
