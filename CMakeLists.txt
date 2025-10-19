cmake_minimum_required(VERSION 3.16)
project(GoGame LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Point to SFML subdirectory
add_subdirectory(external/SFML)

# Add include directory
include_directories(${PROJECT_SOURCE_DIR}/include)

# Add your source files
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/src/*.cpp")

add_executable(${PROJECT_NAME} ${SOURCES})

# Link SFML (graphics, window, system)
target_link_libraries(${PROJECT_NAME} PRIVATE
    sfml-graphics
    sfml-window
    sfml-system
)

# Optionally specify where to output the binary
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build
)

# macOS: ensure frameworks and rpaths are set
#if(APPLE)
#    set_target_properties(${PROJECT_NAME} PROPERTIES
#        MACOSX_BUNDLE TRUE
#        MACOSX_BUNDLE_GUI_IDENTIFIER my.sfml.app
#        RESOURCE "${CMAKE_SOURCE_DIR}/resources"
#    )
#endif()

