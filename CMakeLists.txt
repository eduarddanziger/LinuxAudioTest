cmake_minimum_required (VERSION 3.29)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("AudioTest")

# Pre-configure step: Run pre-configure.cmake
# execute_process(
#     COMMAND ${CMAKE_COMMAND} -P ${CMAKE_SOURCE_DIR}/pre-configure.cmake
#     WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
#     RESULT_VARIABLE pre_configure_result
# )

# Check if the pre-configure step succeeded
# if(NOT pre_configure_result EQUAL 0)
#     message(FATAL_ERROR "Pre-configure step failed with exit code ${pre_configure_result}")
# else()
#     message(STATUS "Running pre-configure step successfull")
# endif()

# Add source to this project's executable.
add_executable (AudioTest "AudioTest.cpp" "AudioTest.h")

target_include_directories(AudioTest PRIVATE "/usr/include/alsa")
target_link_libraries(AudioTest PRIVATE asound)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET AudioTest PROPERTY CXX_STANDARD 20)
endif()

# Find and link fmt library
find_package(fmt REQUIRED)
target_link_libraries(AudioTest PRIVATE fmt::fmt)
