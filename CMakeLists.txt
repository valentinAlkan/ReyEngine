cmake_minimum_required(VERSION 3.11) # FetchContent is available in 3.11+
project(ReyEngine)

set(CMAKE_REQUIRED_FLAGS "-Wno-old-style-definitions")

# add lua
add_subdirectory(src/thirdParty/lua)

# enable -std=c++17
set (CMAKE_CXX_STANDARD 17)

# Add static linking flags for GCC and standard C++ libraries
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++ -static -pthread")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++ -static")

# Ensure pthread is linked statically
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)


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

#set up eigen
message(CHECK_START "Fetching Eigen3")
list(APPEND CMAKE_MESSAGE_INDENT "  ")

include(FetchContent)
FetchContent_Declare(
        Eigen
        GIT_REPOSITORY https://gitlab.com/libeigen/eigen.git
        GIT_TAG master
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE)
# note: To disable eigen tests,
# you should put this code in a add_subdirectory to avoid to change
# BUILD_TESTING for your own project too since variables are directory
# scoped
set(BUILD_TESTING OFF)
set(EIGEN_BUILD_TESTING OFF)
set(EIGEN_MPL2_ONLY ON)
set(EIGEN_BUILD_PKGCONFIG OFF)
set(EIGEN_BUILD_DOC OFF)
FetchContent_MakeAvailable(Eigen)

list(POP_BACK CMAKE_MESSAGE_INDENT)
message(CHECK_PASS "fetched")
# /eigen

set(CUSTOM_SOURCES
    test/TestWidgets.cpp
    test/TextureTestWidget.hpp
    test/RootWidget.hpp
)

add_library(ReyEngine2D STATIC
    src/meta/AStar.cpp
    src/meta/ArgParse.cpp
    src/meta/FileSystem.cpp
    src/meta/StringTools.cpp
    src/meta/MathTools.cpp
    src/meta/DrawInterface.cpp
    src/meta/Application.cpp
    src/meta/Thread.cpp
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
    src/meta/CSVParser.cpp
    src/meta/Platform.cpp
    src/2d/BaseWidget.cpp
    src/2d/ComboBox.cpp
    src/2d/Control.cpp
    src/2d/Canvas.cpp
    src/2d/Panel.cpp
    src/2d/TabContainer.cpp
    src/2d/Slider.hpp
    src/2d/Button.cpp
    src/2d/Label.hpp
    src/2d/Sprite.cpp
    src/2d/Layout.cpp
    src/2d/ScrollArea.cpp
    src/2d/Tree.cpp
    src/2d/TileMap.cpp
    src/2d/LineEdit.cpp
    src/2d/TextureRect.cpp
    src/2d/CustomTypes.cpp
    src/2d/Camera2D.cpp
    src/Pathfinding/SearchNode.cpp
    src/Pathfinding/AStar.cpp
    ${CUSTOM_SOURCES}
)

add_library(ReyEngine3D STATIC
    src/3d/DrawInterface3D.cpp
    src/3d/Viewport.cpp
    src/3d/BaseBody.cpp
    src/3d/Camera3D.cpp
    src/3d/Plane3D.cpp
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
    src/thirdParty/uuid
)

# most stuff is linked against and included with ReyEngine2D already. Since ReyEngine3D is linked against ReyEngine2D,
# there's no need to link against much new stuff (for now)
target_include_directories( ReyEngine3D PUBLIC
    src/3d
)

target_link_libraries(ReyEngine2D raylib lua Threads::Threads Eigen3::Eigen)

if (UNIX)
    target_link_libraries(ReyEngine2D stdc++fs)
endif (UNIX)

target_link_libraries(ReyEngine3D ReyEngine2D)


target_include_directories(editor PUBLIC
    src/editor
)

target_link_libraries(editor PUBLIC ReyEngine2D ReyEngine3D)

# Test Project
set(TEST_PROJECT ReyEngineTest)
add_executable( ${TEST_PROJECT}
    test/reyTestMain.cpp
)

target_link_libraries(${TEST_PROJECT}
    ReyEngine2D
    ReyEngine3D
    editor
)

##########################################
# for testing basic raylib stuff
add_executable( raylibTest
    test/raylibTest.cpp
)
target_link_libraries(raylibTest
    ReyEngine2D
)
add_executable( cppTest
    test/cppTest.cpp
)

##########################################


install_file(./test/test.scn ./test/test.scn)
install_file(./test/posTest.scn ./test/posTest.scn)
install_file(./test/posTest.scn ./test/serializeTest.scn)
install_file(./test/characters.png ./test/characters.png)
install_file(./test/test.lua ./test/test.lua)
install_file(./test/xmltest.xml ./test/xmltest.xml)
install_file(./test/spritesheet.png ./test/spritesheet.png)
install_file(./test/csvTest.csv ./test/csvTest.csv)