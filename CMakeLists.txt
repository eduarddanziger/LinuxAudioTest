cmake_minimum_required(VERSION 3.29)

set(CMAKE_CXX_COMPILER "/usr/bin/g++")

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project("AudioTest")

# Add source to this project's executable.
add_executable(AudioTest "AudioTest.cpp" "AudioTest.h")

# Enforce out-of-source builds
if ("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR "In-source builds are not allowed. Please create a separate build directory.")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_BUILD_TYPE Debug)

find_package(fmt REQUIRED)

# Include directories
target_include_directories(AudioTest PRIVATE "/usr/include/alsa" "/usr/include/fmt")

# Link libraries
target_link_libraries(AudioTest PRIVATE asound fmt::fmt)

# Set C++ standard
if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET AudioTest PROPERTY CXX_STANDARD 20)
endif()

# Set the package name and version
set(CPACK_PACKAGE_NAME "AudioTest")
set(CPACK_PACKAGE_VERSION "1.0.0")

# Set the package contact information
set(CPACK_PACKAGE_CONTACT "Eduard Danziger <edanziger@gmx.de>")

# Specify the package type (e.g., DEB)
if(UNIX AND NOT APPLE)
  set(CPACK_GENERATOR "DEB")
endif()

# Specify the installation directory
set(CPACK_PACKAGE_INSTALL_DIRECTORY "AudioTest")

# Specify the license file
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")

# Specify the readme file
set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README.md")

# Specify the description
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "AudioTest is a test application for audio processing.")

# Specify the dependencies (for DEB packages)
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libasound2, libfmt-dev")

# Specify the installation paths
install(TARGETS AudioTest DESTINATION bin)

# Include CPack to enable packaging
include(CPack)

# Commented out the custom target for creating a DEB package
# add_custom_target(
#     package_deb ALL
#     COMMAND ${CMAKE_CPACK_COMMAND} -G DEB
#     DEPENDS AudioTest
# )

