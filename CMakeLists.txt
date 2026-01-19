cmake_minimum_required(VERSION 3.16)
project(GoGame LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Point to SFML subdirectory
add_subdirectory(external/SFML)

# Add include directory
include_directories(${PROJECT_SOURCE_DIR}/include)

# Add source files
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    "${PROJECT_SOURCE_DIR}/src/*.cpp"
    "${PROJECT_SOURCE_DIR}/src/AI_Utils/*.cpp"
)

# Add assets and save files
file(GLOB_RECURSE RESOURCES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/assets"
    "${CMAKE_SOURCE_DIR}/saves"
)

# Create the executable
add_executable(${PROJECT_NAME}
    # MACOSX_BUNDLE
    ${SOURCES}
)

# Link SFML (graphics, window, system)
target_link_libraries(${PROJECT_NAME} PRIVATE
    sfml-graphics
    sfml-window
    sfml-system
    sfml-audio
)

# Optionally specify where to output the binary
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build
)


target_compile_definitions(GoGame PRIVATE ASSET_DIR="${CMAKE_SOURCE_DIR}/assets")
target_compile_definitions(GoGame PRIVATE SAVEGAME_DIR="${CMAKE_SOURCE_DIR}/saves")


# add_custom_target(copy_assets
#     COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_LIST_DIR}/copy_assets.cmake
# )
# add_dependencies(GoGame copy_assets)


# macOS: ensure frameworks and rpaths are set
# if(APPLE)
#     set_target_properties(${PROJECT_NAME} PROPERTIES
#         MACOSX_BUNDLE TRUE
#         MACOSX_BUNDLE_BUNDLE_NAME ${PROJECT_NAME}
#         MACOSX_BUNDLE_GUI_IDENTIFIER my.sfml.app
#         MACOSX_BUNDLE_BUNDLE_VERSION "1.0.0"
#         MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0"
#         RESOURCE "${CMAKE_SOURCE_DIR}/resources"
#     )
#     # Place the file into .app/Contents/Resources
#     set_source_files_properties(${RESOURCES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
# endif()

