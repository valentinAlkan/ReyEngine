cmake_minimum_required(VERSION 3.16)
project(ReyEngine VERSION 0.1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Component options
option(REY_ENABLE_NETWORK "Enable the Network component" ON)
option(REY_ENABLE_YAML "Enable YAML support" ON)
option(REY_ENABLE_LUA "Enable LUA support" ON)

# Ensure pthread is linked statically
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)


function(install_file SOURCE_PATH DESTINATION_PATH)
    # Use STATUS for informational messages during configuration
    message(STATUS "Installing file '${SOURCE_PATH}' to '${DESTINATION_PATH}'")

    # Use the 'install' command for proper installation
    # SOURCE_PATH should be relative to CMAKE_CURRENT_SOURCE_DIR, or an absolute path
    # DESTINATION_PATH is relative to CMAKE_INSTALL_PREFIX
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_PATH}" DESTINATION "${DESTINATION_PATH}")

    # Optional: Add permissions if needed (e.g., for scripts or config files)
    # install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_PATH}"
    #         DESTINATION "${DESTINATION_PATH}"
    #         PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
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

# set up yaml-cpp
if(REY_ENABLE_YAML)
    include(FetchContent)
    FetchContent_Declare(
            yaml-cpp
            GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
            GIT_TAG 0.8.0
    )
    # turn off yaml build targets
    set(YAML_CPP_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
    set(YAML_CPP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(YAML_CPP_BUILD_CONTRIB OFF CACHE BOOL "" FORCE)
    set(YAML_CPP_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(yaml-cpp)
    set(YAML_AVAILABLE TRUE)
endif()

#####################################################################################

set(CUSTOM_SOURCES
    test/TestWidgets.cpp
    test/TextureTestWidget.hpp
    test/RootWidget.hpp
)

add_library(ComponentLogger STATIC src/components/Logger.cpp)
target_include_directories(ComponentLogger PUBLIC src/components)

add_library(ComponentStringTools STATIC src/components/StringTools.cpp)
target_include_directories(ComponentStringTools PUBLIC src/components)

add_library(ComponentFilesystem STATIC src/components/FileSystem.cpp)
target_include_directories(ComponentFilesystem PUBLIC src/components)

add_library(ComponentNetwork STATIC
    src/components/network/Network.cpp
    src/components/network/UDP.cpp
    src/components/network/TCP.cpp
    src/components/Platform.cpp
    src/components/CircBuf.cpp
    src/components/SystemTime.cpp
)
target_link_libraries(ComponentNetwork PUBLIC ComponentLogger)
target_include_directories(ComponentNetwork PUBLIC
    src/components/network
    src/components
)

#link platform specific libraries
if(WIN32)
    target_link_libraries(ComponentNetwork PUBLIC ws2_32)
ELSE()
#    target_link_libraries(core PUBLIC dl)
ENDIF()

##### ReyEngine2D
add_library(ReyEngine2D STATIC
        src/engine/Canvas.cpp
        src/engine/Widget.cpp
        src/engine/Positionable2D.cpp
        src/engine/Window.cpp
        src/engine/WindowPrototype.cpp
        src/engine/Event.cpp
        src/engine/InputManager.cpp
        src/engine/Application.cpp
        src/engine/ReyEngine.cpp
        src/engine/Processable.cpp

        src/components/SystemTime.cpp
        src/components/MetaData.cpp

        src/2d/Control.cpp
        src/2d/Layout.cpp
        src/2d/Button.cpp
        src/2d/LineEdit.cpp
        src/2d/TabContainer.cpp
        src/2d/ScrollArea.cpp
        src/2d/TextureRect.cpp
        src/2d/Sprite.cpp
        src/2d/TileMap.cpp
)

target_include_directories(ReyEngine2D PUBLIC
    src
    src/engine
    src/meta
    src/2d
    src/lib
    test
    src/thirdParty/xml
    src/thirdParty/uuid
    src/pathfinding
    src/physics
)
target_compile_features(ReyEngine2D PUBLIC cxx_std_20)
target_compile_options(ReyEngine2D PUBLIC -pthread)

#conditional compilations
if(REY_ENABLE_LUA)
    add_subdirectory(src/thirdParty/lua)
    target_link_libraries(ReyEngine2D lua)
endif()
if(REY_ENABLE_YAML)
    target_link_libraries(ReyEngine2D yaml-cpp::yaml-cpp)
endif()

#default compilations
target_link_libraries(ReyEngine2D raylib Threads::Threads ComponentStringTools ComponentFilesystem ComponentLogger)


if (UNIX)
    target_link_libraries(ReyEngine2D stdc++fs)
endif (UNIX)

##### TILE EDITOR
add_executable(TileEditor
    apps/TileEditor/src/main.cpp
    apps/TileEditor/src/UI.cpp
    apps/TileEditor/src/TileEditor.cpp
)

target_include_directories(TileEditor PUBLIC apps/TileEditor/src)
target_link_libraries(TileEditor ReyEngine2D)

#add_library(ReyEngine3D STATIC
#    src/3d/DrawInterface3D.cpp
#    src/3d/Viewport.cpp
#    src/3d/BaseBody.cpp
#    src/3d/Camera3D.cpp
#    src/3d/Plane3D.cpp
#    src/3d/ModelBody.cpp
#)

#add_library(Editor STATIC
#    src/editor/Editor.cpp
#    src/editor/WidgetTree.cpp
#    src/editor/SceneTree.cpp
#    src/editor/Workspace.cpp
#    src/editor/Inspector.cpp
#    src/editor/InspectorWidgetFactory.cpp
#)

# most stuff is linked against and included with ReyEngine2D already. Since ReyEngine3D is linked against ReyEngine2D,
# there's no need to link against much new stuff (for now)
#target_include_directories( ReyEngine3D PUBLIC
#    src/3d
#)




#target_link_libraries(ReyEngine3D ReyEngine2D)


#target_include_directories(Editor PUBLIC
#    src/editor
#)

#target_link_libraries(Editor PUBLIC ReyEngine2D ReyEngine3D)

# Test Project
#add_executable( ReyEngineTest test/reyTestMain.cpp)
#target_link_libraries(ReyEngineTest Editor)

# Test Project
#add_executable( AbstractTreeTest test/treeTest.cpp src/meta/Logger.cpp)
#target_include_directories(AbstractTreeTest PUBLIC src/lib src/meta)
#target_link_libraries(AbstractTreeTest raylib)

# Test Project
add_executable( NewEngineMain test/newEngine/newEngineTestMain.cpp)
target_link_libraries(NewEngineMain ReyEngine2D)
add_executable( NewEngineBasic test/newEngine/NewEngineTestBasic.cpp)
target_link_libraries(NewEngineBasic ReyEngine2D)


# Test Project
add_executable( NewEngineNetwork test/newEngine/newEngineTestNetwork.cpp)
target_link_libraries(NewEngineNetwork ReyEngine2D ComponentNetwork)

# Project for testing the build system
#add_executable( ReyEngineBuildTest test/buildTest.cpp)
#target_link_libraries(ReyEngineBuildTest ReyEngine2D)

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


install_file(test/test.scn test/)
install_file(test/posTest.scn test/)
install_file(test/posTest.scn test/)
install_file(test/characters.png test/)
install_file(test/test.lua test/)
install_file(test/xmltest.xml test/)
install_file(test/spritesheet.png test/)
install_file(test/csvTest.csv test/)
install_file(test/3d/suzanne.obj test/)
install_file(test/10x10_checkered_board.png test/)
install_file(test/tiles.png test/)