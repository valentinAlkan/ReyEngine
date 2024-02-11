cmake_minimum_required(VERSION 3.11) # FetchContent is available in 3.11+
project(ReyEngine)

# Generate compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# enable -std=c++17
set (CMAKE_CXX_STANDARD 17)

function(install_file SOURCE DEST)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE} ${CMAKE_CURRENT_BINARY_DIR}/${DEST} COPYONLY)
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


set(CUSTOM_SOURCES
    widgets/test/TestWidgets.cpp
    widgets/test/TextureTestWidget.hpp
    widgets/test/RootWidget.hpp
)

add_library(widgets STATIC
    widgets/meta/ArgParse.cpp
    widgets/meta/FileSystem.cpp
    widgets/meta/StringTools.cpp
    widgets/meta/MathTools.cpp
    widgets/meta/DrawInterface.cpp
    widgets/meta/Application.cpp
    widgets/meta/TypeManager.cpp
    widgets/meta/Window.cpp
    widgets/meta/Scene.cpp
    widgets/meta/Property.cpp
    widgets/meta/Logger.cpp
    widgets/meta/SystemTime.cpp
    widgets/meta/Event.cpp
    widgets/meta/InputManager.cpp
    widgets/meta/Timer.hpp
    widgets/meta/theme.cpp
    widgets/meta/Font.hpp
    widgets/meta/Component.cpp
    widgets/BaseWidget.cpp
    widgets/Control.hpp
    widgets/Canvas.hpp
    widgets/Panel.hpp
    widgets/Slider.hpp
    widgets/Button.hpp
    widgets/Label.hpp
    widgets/Sprite.cpp
    widgets/Layout.hpp
    widgets/ScrollArea.hpp
    widgets/Tree.cpp
    ${CUSTOM_SOURCES}
)

add_library(widgets3d STATIC
    widgets/3d/Viewport.cpp
)

add_library(editor STATIC
    editor/Editor.cpp
    editor/WidgetTree.cpp
    editor/SceneTree.cpp
    editor/Workspace.cpp
    editor/Inspector.cpp
    editor/InspectorWidgetFactory.cpp
)

target_include_directories(widgets PUBLIC
    widgets
    widgets/meta
    widgets/test
)

target_link_libraries(widgets raylib)

if (UNIX)
    target_link_libraries(widgets stdc++fs)
endif (UNIX)


target_include_directories(editor PUBLIC
    editor
)

target_link_libraries(editor PUBLIC widgets)

# Test Project
set(TEST_PROJECT ReyTest)
add_executable( ${TEST_PROJECT}
        widgets/test/reyTestMain.cpp
)

target_link_libraries(${TEST_PROJECT}
    widgets
    widgets3d
    editor
)

install_file(./widgets/test/test.scn ./test/test.scn)
install_file(./widgets/test/posTest.scn ./test/posTest.scn)
install_file(./widgets/test/posTest.scn ./test/serializeTest.scn)
install_file(./widgets/test/characters.png ./test/characters.png)