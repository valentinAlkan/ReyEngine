cmake_minimum_required(VERSION 3.11) # FetchContent is available in 3.11+
project(ReyEngine)

# add lua
add_subdirectory(src/thirdParty/lua)

# Generate compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# enable -std=c++17
set (CMAKE_CXX_STANDARD 17)

function(install_file SOURCE DEST)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${DEST} COPYONLY)
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
    test/TestWidgets.cpp
    test/TextureTestWidget.hpp
    test/RootWidget.hpp
)

add_library(ReyEngine2D STATIC
    src/meta/ArgParse.cpp
    src/meta/FileSystem.cpp
    src/meta/StringTools.cpp
    src/meta/MathTools.cpp
    src/meta/DrawInterface.cpp
    src/meta/Application.cpp
    src/meta/TypeManager.cpp
    src/meta/Window.cpp
    src/meta/Scene.cpp
    src/meta/Property.cpp
    src/meta/Logger.cpp
    src/meta/XML.cpp
    src/meta/SystemTime.cpp
    src/meta/Event.cpp
    src/meta/InputManager.cpp
    src/meta/Timer.hpp
    src/meta/Theme.cpp
    src/meta/Font.hpp
    src/meta/Component.cpp
    src/meta/Config.cpp
    src/2d/BaseWidget.cpp
    src/2d/ComboBox.cpp
    src/2d/Control.cpp
    src/2d/Canvas.cpp
    src/2d/Panel.hpp
    src/2d/TabContainer.cpp
    src/2d/Slider.hpp
    src/2d/Button.cpp
    src/2d/Label.hpp
    src/2d/Sprite.cpp
    src/2d/Layout.cpp
    src/2d/ScrollArea.hpp
    src/2d/Tree.cpp
    src/2d/TileMap.cpp
    src/2d/LineEdit.cpp
    ${CUSTOM_SOURCES}
)

add_library(ReyEngine3D STATIC
    src/3d/Viewport.cpp
)

add_library(editor STATIC
    src/editor/Editor.cpp
    src/editor/WidgetTree.cpp
    src/editor/SceneTree.cpp
    src/editor/Workspace.cpp
    src/editor/Inspector.cpp
    src/editor/InspectorWidgetFactory.cpp
)

target_include_directories(ReyEngine2D PUBLIC
    src
    src/meta
    src/2d
    test
    src/thirdParty/xml
)

target_link_libraries(ReyEngine2D raylib lua)

if (UNIX)
    target_link_libraries(ReyEngine2D stdc++fs)
endif (UNIX)


target_include_directories(editor PUBLIC
    src/editor
)

target_link_libraries(editor PUBLIC ReyEngine2D ReyEngine3D)

# Test Project
set(TEST_PROJECT ReyTest)
add_executable( ${TEST_PROJECT}
        test/reyTestMain.cpp
)

target_link_libraries(${TEST_PROJECT}
    ReyEngine2D
    ReyEngine3D
    editor
)

install_file(./test/test.scn ./test/test.scn)
install_file(./test/posTest.scn ./test/posTest.scn)
install_file(./test/posTest.scn ./test/serializeTest.scn)
install_file(./test/characters.png ./test/characters.png)
install_file(./test/test.lua ./test/test.lua)
install_file(./test/xmltest.xml ./test/xmltest.xml)