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
    test/TestWidgets.cpp
    test/TextureTestWidget.hpp
    test/RootWidget.hpp
)

###################################
#ReyMeta
###################################
add_library(ReyMeta STATIC
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
    src/meta/SystemTime.cpp
    src/meta/Event.cpp
    src/meta/InputManager.cpp
    src/meta/Timer.hpp
    src/meta/Theme.cpp
    src/meta/Font.hpp
    src/meta/IndexedMap.cpp
)

target_include_directories(ReyMeta PUBLIC
    src/meta
)

target_link_libraries(ReyMeta PUBLIC
    raylib
)
##
####################################
##Rey2D
####################################
#add_library(Rey2D STATIC
#    src/Rey2d/BaseWidget.cpp
#    src/Rey2d/Control.hpp
#    src/Rey2d/Canvas.hpp
#    src/Rey2d/Panel.hpp
#    src/Rey2d/Slider.hpp
#    src/Rey2d/Button.hpp
#    src/Rey2d/Label.hpp
#    src/Rey2d/Sprite.cpp
#    src/Rey2d/Layout.hpp
#    src/Rey2d/ScrollArea.hpp
#    src/Rey2d/Tree.cpp
#    ${CUSTOM_SOURCES}
#)
#
#target_include_directories(Rey2D PUBLIC
#    src/Rey2d
#)
#
#target_link_libraries(Rey2D PUBLIC
#    ReyMeta
#)
#
#add_library(Rey3D STATIC
#    src/Rey3d/Camera3D.cpp
#)
#
#add_library(editor STATIC
#    src/editor/Editor.cpp
#    src/editor/WidgetTree.cpp
#    src/editor/SceneTree.cpp
#    src/editor/Workspace.cpp
#    src/editor/Inspector.cpp
#    src/editor/InspectorWidgetFactory.cpp
#)
#
#
#
#target_link_libraries(ReyEngine raylib)
#
#if (UNIX)
#    target_link_libraries(Rey2D stdc++fs)
#    target_link_libraries(Rey3D stdc++fs)
#endif (UNIX)
#
#
#target_include_directories(editor PUBLIC
#    editor
#)
#
#target_link_libraries(editor PUBLIC widgets)
#
## Test Project
#set(TEST_PROJECT ReyTest)
#add_executable( ${TEST_PROJECT}
#    test/reyTestMain.cpp
#)
#
#target_link_libraries(${TEST_PROJECT}
#    widgets
#    editor
#)
#
#install_file(./test/test.scn ./test/test.scn)
#install_file(./test/posTest.scn ./test/posTest.scn)
#install_file(./test/posTest.scn ./test/serializeTest.scn)
#install_file(./test/characters.png ./test/characters.png)