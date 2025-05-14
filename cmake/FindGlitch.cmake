# FindGlitch.cmake - A CMake module to find the Glitch library

# Try to find an installed version of Glitch
find_path(GLITCH_INCLUDE_DIR glitch/pch.h
    PATHS ${CMAKE_INSTALL_PREFIX}/include
    PATH_SUFFIXES glitch
)

find_path(IMGUI_INCLUDE_DIR imgui/imgui.h
    PATHS ${CMAKE_INSTALL_PREFIX}/include
    PATH_SUFFIXES imgui
)

find_library(GLITCH_LIBRARY
    NAMES glitch
    PATHS ${CMAKE_INSTALL_PREFIX}/lib
)

# Provide information about the found package
if(GLITCH_INCLUDE_DIR
    AND STB_INCLUDE_DIR
    AND IMGUI_INCLUDE_DIR
    AND GLITCH_LIBRARY)
    set(GLITCH_FOUND TRUE)
else()
    set(GLITCH_FOUND FALSE)
endif()

# If Glitch is found, provide the necessary variables
if(GLITCH_FOUND)
    set(GLITCH_INCLUDE_DIRS
        ${GLITCH_INCLUDE_DIR}
        ${IMGUI_INCLUDE_DIR}
    )
    set(GLITCH_LIBRARIES ${GLITCH_LIBRARY})
    
    message(STATUS "Found Glitch: ${GLITCH_LIBRARY} in ${GLITCH_INCLUDE_DIR}")
else()
    message(STATUS "Glitch not found.")
endif()

# Allow to link Glitch in the user's project
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Glitch REQUIRED_VARS
    GLITCH_INCLUDE_DIR GLITCH_LIBRARY
)
