cmake_minimum_required(VERSION 3.11) # FetchContent is available in 3.11+
project(lua)

# Generate compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# enable -std=c++17
set (CMAKE_CXX_STANDARD 17)

# Dependencies
add_library(lua STATIC
    lapi.c
    lauxlib.c
    lcode.c
    lctype.c
    lbaselib.c
    lcorolib.c
    ldblib.c
    ldo.c
    ldump.c
    lfunc.c
    lgc.c
    linit.c
    liolib.c
    llex.c
    lmathlib.c
    lmem.c
    loadlib.c
    lobject.c
    lopcodes.c
    loslib.c
    lparser.c
    lstate.c
    lstring.c
    lstrlib.c
    ltable.c
    ltablib.c
    ltm.c
    lua.c
    luac.c
    lundump.c
    lutf8lib.c
    lvm.c
    lzio.c
)

target_include_directories(lua PUBLIC .)