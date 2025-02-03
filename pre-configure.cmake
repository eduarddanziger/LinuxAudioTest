# pre-configure.cmake
message(STATUS "Running pre-configure script...")
execute_process(
    COMMAND powershell.exe -File "${CMAKE_SOURCE_DIR}/GetWslEthernetAdapter.ps1"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)