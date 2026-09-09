/*
 Copyright (c) 2026 ETIB Corporation

 Permission is hereby granted, free of charge, to any person obtaining a copy of
 this software and associated documentation files (the "Software"), to deal in
 the Software without restriction, including without limitation the rights to
 use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 of the Software, and to permit persons to whom the Software is furnished to do
 so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
 */

#pragma once

#include <android_native_app_glue.h>
#include <evan/openxr/platform/AndroidXrPlatform.hpp>
#include <utility/logging/loggable.hpp>
#include <utility/logging/default_logger.hpp>

#include <functional>

/**
 * @brief Handles Android application lifecycle commands.
 *
 * Encapsulates the native-app-glue command processing into a class that
 * inherits from utility::logging::Loggable so all state transitions are
 * logged through the platform default logger (AndroidLogger on Android).
 */
class CommandHandler:
	protected utility::logging::Loggable<CommandHandler,
										 utility::logging::DefaultLogger>
{
	private:
	evan::AndroidXrPlatform::AndroidAppState *_appState = nullptr;

	public:
	/**
	 * @brief Link the handler to the application's state struct.
	 * @param state Pointer to the AndroidAppState maintained by the platform.
	 */
	void setAppState(evan::AndroidXrPlatform::AndroidAppState *state);

	/**
	 * @brief Process a single Android app command.
	 * @param android_app  The android_app structure provided by the framework.
	 * @param cmd  The command id (APP_CMD_* constants).
	 */
	void handle(struct android_app *android_app, int32_t cmd);

	/**
	 * @brief Set a callback to be invoked when the app receives the
	 * APP_CMD_PAUSE command.
	 * @param callback A function to call when the app is paused.
	 */
	void setOnPauseCallback(std::function<void()> callback);

	private:
	/**
	 * @brief Handle the APP_CMD_PAUSE command.
	 *
	 * This method is called when the Android application receives the
	 * APP_CMD_PAUSE command, indicating that the app is being paused. It
	 * updates the internal state of the AndroidAppState struct to reflect
	 * that the app is now paused and not resumed.
	 */
	std::function<void()> onPause;
};
