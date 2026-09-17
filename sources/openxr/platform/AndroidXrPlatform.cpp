/*
** ETIB PROJECT, 2026
** evan
** File description:
** AndroidXrPlatform
*/

#include "evan/openxr/platform/AndroidXrPlatform.hpp"
#include "evan/openxr/XrDeviceBackend.hpp"

#ifdef __ANDROID__
	#include <jni.h>
#endif

namespace
{
#ifdef __ANDROID__
	/**
	 * @brief Calls `Context.getCacheDir()` on a Java activity.
	 *
	 * The caller must have pushed a local frame beforehand: every local
	 * reference created here is released by the matching `PopLocalFrame`.
	 *
	 * @param environment JNI environment of the calling thread.
	 * @param activity The Java activity object.
	 * @return The absolute path of the cache directory, or an empty path when
	 * the Java method is unavailable.
	 */
	std::filesystem::path queryJavaCacheDirectory(JNIEnv *environment,
												 jobject activity)
	{
		jclass contextClass = environment->GetObjectClass(activity);
		if (contextClass == nullptr) {
			return {};
		}

		jmethodID getCacheDir = environment->GetMethodID(
			contextClass, "getCacheDir", "()Ljava/io/File;");
		if (getCacheDir == nullptr || environment->ExceptionCheck()) {
			return {};
		}

		jobject cacheFile =
			environment->CallObjectMethod(activity, getCacheDir);
		if (cacheFile == nullptr) {
			return {};
		}

		jclass fileClass = environment->GetObjectClass(cacheFile);
		if (fileClass == nullptr) {
			return {};
		}

		jmethodID getAbsolutePath = environment->GetMethodID(
			fileClass, "getAbsolutePath", "()Ljava/lang/String;");
		if (getAbsolutePath == nullptr || environment->ExceptionCheck()) {
			return {};
		}

		auto path = reinterpret_cast<jstring>(
			environment->CallObjectMethod(cacheFile, getAbsolutePath));
		if (path == nullptr) {
			return {};
		}

		const char *characters = environment->GetStringUTFChars(path, nullptr);
		if (characters == nullptr) {
			return {};
		}

		std::filesystem::path directory(characters);
		environment->ReleaseStringUTFChars(path, characters);
		return directory;
	}

	/**
	 * @brief Resolves the Java cache directory of an Android activity.
	 *
	 * The platform data only carries opaque pointers to the Java VM and to the
	 * activity object, so the application cache directory is asked to the
	 * activity through JNI (`Context.getCacheDir()`).
	 *
	 * Every failure (unusable VM, missing activity, pending Java exception)
	 * returns an empty path so the caller falls back to not persisting.
	 *
	 * @param applicationVM Pointer to the Java VM (`JavaVM *`).
	 * @param activity Pointer to the Java activity object (`jobject`).
	 * @return The absolute path of the application cache directory, or an
	 * empty path.
	 */
	std::filesystem::path getAndroidCacheDirectory(void *applicationVM,
												   void *activity)
	{
		if (applicationVM == nullptr || activity == nullptr) {
			return {};
		}

		auto *vm = static_cast<JavaVM *>(applicationVM);
		auto *instance = static_cast<jobject>(activity);
		JNIEnv *environment = nullptr;
		bool attached = false;

		if (vm->GetEnv(reinterpret_cast<void **>(&environment),
					   JNI_VERSION_1_6)
			!= JNI_OK) {
			if (vm->AttachCurrentThread(&environment, nullptr) != JNI_OK) {
				return {};
			}
			attached = true;
		}

		std::filesystem::path directory;
		if (environment->PushLocalFrame(16) == JNI_OK) {
			directory = queryJavaCacheDirectory(environment, instance);
			environment->PopLocalFrame(nullptr);
		}

		if (environment->ExceptionCheck()) {
			environment->ExceptionClear();
			directory.clear();
		}

		if (attached) {
			vm->DetachCurrentThread();
		}

		return directory;
	}
#endif
}	 // namespace

evan::AndroidXrPlatform::AndroidXrPlatform(
	const AndroidPlatformData &platformData)
{
	this->getLogger().info() << "Initializing AndroidXrPlatform with provided "
								"Android platform data.";
	_platformData = platformData;
#ifdef __ANDROID__
	PFN_xrInitializeLoaderKHR initializeLoader = nullptr;

	if (xrGetInstanceProcAddr(
			XR_NULL_HANDLE, "xrInitializeLoaderKHR",
			reinterpret_cast<PFN_xrVoidFunction *>(&initializeLoader))
		== XR_SUCCESS) {
		XrLoaderInitInfoAndroidKHR loaderInitInfoAndroid = {};
		loaderInitInfoAndroid.type = XR_TYPE_LOADER_INIT_INFO_ANDROID_KHR;
		loaderInitInfoAndroid.next = nullptr;
		loaderInitInfoAndroid.applicationVM = platformData.applicationVM;
		loaderInitInfoAndroid.applicationContext =
			platformData.applicationActivity;
		initializeLoader(
			reinterpret_cast<const XrLoaderInitInfoBaseHeaderKHR *>(
				&loaderInitInfoAndroid));
	}
	_instanceCreateInfoAndroid = { XR_TYPE_INSTANCE_CREATE_INFO_ANDROID_KHR };
	_instanceCreateInfoAndroid.applicationVM = platformData.applicationVM;
	_instanceCreateInfoAndroid.applicationActivity =
		platformData.applicationActivity;
#endif
}

////////////////////
// Public Methods //
////////////////////

std::vector<std::string>
	evan::AndroidXrPlatform::getRequiredInstanceExtensions() const
{
	std::vector<std::string> extensions = {
		XR_KHR_VULKAN_ENABLE2_EXTENSION_NAME,
#ifdef XR_USE_PLATFORM_ANDROID
		XR_KHR_ANDROID_CREATE_INSTANCE_EXTENSION_NAME
#endif
	};
	return extensions;
}

const XrBaseInStructure *evan::AndroidXrPlatform::getInstanceCreateInfo() const
{
#ifdef __ANDROID__
	return reinterpret_cast<const XrBaseInStructure *>(
		&_instanceCreateInfoAndroid);
#else
	return nullptr;
#endif
}

///////////////////////
// Protected Methods //
///////////////////////

std::filesystem::path evan::AndroidXrPlatform::getDefaultCacheRoot() const
{
#ifdef __ANDROID__
	return getAndroidCacheDirectory(_platformData.applicationVM,
									_platformData.applicationActivity);
#else
	return {};
#endif
}
