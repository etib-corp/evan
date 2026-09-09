set(ANDROID_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(GRADLE_TASK "assembleDebug")
  set(GRADLE_VARIANT "debug")
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
  set(GRADLE_TASK "assembleRelease")
  set(GRADLE_VARIANT "release")
elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
  set(GRADLE_TASK "assembleRelease")
  set(GRADLE_VARIANT "release")
elseif(CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
  set(GRADLE_TASK "assembleRelease")
  set(GRADLE_VARIANT "release")
else()
  set(GRADLE_TASK "assembleDebug")
  set(GRADLE_VARIANT "debug")
endif()

if(WIN32)
  set(GRADLEW "${ANDROID_DIR}/gradlew.bat")
else()
  set(GRADLEW "${ANDROID_DIR}/gradlew")
endif()

set(GRADLE_OUTPUT "${ANDROID_DIR}/app/build/outputs/apk/${GRADLE_VARIANT}/app-${GRADLE_VARIANT}.apk")

add_custom_command(
  OUTPUT "${GRADLE_OUTPUT}"
  COMMAND "${GRADLEW}" "${GRADLE_TASK}"
  WORKING_DIRECTORY "${ANDROID_DIR}"
  BYPRODUCTS "${GRADLE_OUTPUT}"
  COMMENT "Running ${GRADLE_TASK}"
  VERBATIM
)

add_custom_target(android_apk ALL
  DEPENDS "${GRADLE_OUTPUT}"
)
