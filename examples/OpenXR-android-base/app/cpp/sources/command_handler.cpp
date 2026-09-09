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

#include "command_handler.hpp"

void CommandHandler::setAppState(
	evan::AndroidXrPlatform::AndroidAppState *state)
{
	_appState = state;
}

void CommandHandler::handle(struct android_app * /*android_app*/, int32_t cmd)
{
	if (!_appState) {
		this->getLogger().warning() << "handle() called before setAppState()";
		return;
	}

	switch (cmd) {
		case APP_CMD_START: {
			this->getLogger().debug() << "APP_CMD_START onStart()";
			break;
		}
		case APP_CMD_RESUME: {
			this->getLogger().debug() << "APP_CMD_RESUME onResume()";
			_appState->_resumed = true;
			_appState->_paused	= false;
			break;
		}
		case APP_CMD_PAUSE: {
			this->getLogger().debug() << "APP_CMD_PAUSE onPause()";
			_appState->_resumed = false;
			_appState->_paused	= true;
			if (onPause) {
				onPause();
			}
			break;
		}
		case APP_CMD_STOP: {
			this->getLogger().debug() << "APP_CMD_STOP onStop()";
			break;
		}
		case APP_CMD_DESTROY: {
			this->getLogger().debug() << "APP_CMD_DESTROY onDestroy()";
			break;
		}
		case APP_CMD_INIT_WINDOW: {
			this->getLogger().debug() << "APP_CMD_INIT_WINDOW surfaceCreated()";
			break;
		}
		case APP_CMD_TERM_WINDOW: {
			this->getLogger().debug()
				<< "APP_CMD_TERM_WINDOW surfaceDestroyed()";
			break;
		}
	}
}

void CommandHandler::setOnPauseCallback(std::function<void()> callback)
{
	this->onPause = callback;
}