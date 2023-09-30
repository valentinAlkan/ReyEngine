cmake_minimum_required(VERSION 3.11) # FetchContent is available in 3.11+
project(example)

# Generate compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# enable -std=c++17
set (CMAKE_CXX_STANDARD 17)

function(install_file SOURCE DEST)
    add_custom_command(
            TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_SOURCE_DIR}/${SOURCE}
            ${CMAKE_CURRENT_BINARY_DIR}/${DEST})
endfunction()


# Dependencies
set(RAYLIB_VERSION 4.5.0)
find_package(raylib ${RAYLIB_VERSION} QUIET) # QUIET or REQUIRED
if (NOT raylib_FOUND) # If there's none, fetch and build raylib
    include(FetchContent)
    FetchContent_Declare(
            raylib
            DOWNLOAD_EXTRACT_TIMESTAMP OFF
            URL https://github.com/raysan5/raylib/archive/refs/tags/${RAYLIB_VERSION}.tar.gz
    )
    FetchContent_GetProperties(raylib)
    if (NOT raylib_POPULATED) # Have we downloaded raylib yet?
        set(FETCHCONTENT_QUIET NO)
        FetchContent_Populate(raylib)
        set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # don't build the supplied examples
        add_subdirectory(${raylib_SOURCE_DIR} ${raylib_BINARY_DIR})
    endif()
endif()

add_library(widgets STATIC
        widgets/meta/ArgParse.cpp
        widgets/meta/FileSystem.cpp
        widgets/meta/StringTools.cpp
        widgets/meta/DrawInterface.cpp
        widgets/meta/Application.cpp
        widgets/meta/TypeManager.cpp
        widgets/meta/Window.cpp
        widgets/meta/Scene.cpp
        widgets/meta/Property.cpp
        widgets/meta/Logger.cpp
        widgets/meta/SystemTime.cpp
        widgets/Button.cpp
        widgets/BaseWidget.cpp
)

target_include_directories(widgets PUBLIC
        widgets
        widgets/meta
)

target_link_libraries(widgets raylib)
if (UNIX)
    target_link_libraries(widgets stdc++fs)
endif (UNIX)

# Our Project
add_executable(${PROJECT_NAME}
        main.cpp
        )

target_link_libraries(${PROJECT_NAME} widgets)

install_file(./widgets/test/test.scn ./test/test.scn)