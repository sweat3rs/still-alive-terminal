cmake_minimum_required (VERSION 3.10)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 26)
cmake_policy(SET CMP0167 OLD)

set(PROJECT_NAME still-alive-terminal)
set(VERSION "0.1.0")

set(EXECUTABLE_NAME still_alive)

project(${PROJECT_NAME} VERSION ${VERSION} LANGUAGES C CXX)

if (MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra)
endif()

# -------------------------------- Directories --------------------------------

set(LIBRARY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/bin/${CMAKE_SYSTEM_NAME}/${PRESET_NAME}")

set(STATIC_DIR "${LIBRARY_DIR}/lib")
set(SHARED_DIR "${LIBRARY_DIR}/dyn")
set(EXPORT_DIR "${LIBRARY_DIR}/export")
set(INSTALL_DIR "${LIBRARY_DIR}/install")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${STATIC_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${SHARED_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${SHARED_DIR})

set(EXTERN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/extern")
set(MINIAUDIO "${EXTERN_DIR}/miniaudio")

# ------------------------------- Gather Sources --------------------------------
set(SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")

FILE(GLOB SOURCES "${SOURCE_DIR}/*.cpp" "${SOURCE_DIR}/*.c")

# ------------------------------- Platform Guards -------------------------------
# I only allow platforms I have tested on, though you are free to remove this section and try it on something else

if ((NOT (( CMAKE_SYSTEM_NAME STREQUAL "Darwin") OR (CMAKE_SYSTEM_NAME STREQUAL "Windows"))) OR (NOT CMAKE_SIZEOF_VOID_P EQUAL 8))
  message(FATAL_ERROR "${PROJECT_NAME} only supports 64-bit MacOS and Windows systems!")
endif()

# ----------------------------- Dependencies/Packages ----------------------------

set(CURSES_NEED_NCURSES true)
# set(CURSES_NEED_WIDE true)

find_package(Curses REQUIRED)
if (NOT Curses_FOUND)
  message(FATAL_ERROR "Could not find a supported ncurses library. Do you have ncurses installed on your system?")
endif()

find_package(Lua REQUIRED)
if (NOT Lua_FOUND)
  message(FATAL_ERROR "Could not find a supported lua c library. Do you have lua installed on your system?")
endif()

find_package(Boost REQUIRED)
if (NOT Boost_FOUND)
  message(FATAL_ERROR "Could not find a supported boost library. Do you have c++ boost libraries installed on your system?")
endif()

# ----------------------------------- Add EXE -----------------------------------

add_executable(${EXECUTABLE_NAME} ${SOURCES} "${MINIAUDIO}/miniaudio.c")

add_compile_definitions(SOURCE_RUNTIME_DIRECTORY="${CMAKE_CURRENT_SOURCE_DIR}/runtime/")

# ----------------------------- Include Directories -----------------------------

target_include_directories(${EXECUTABLE_NAME} PUBLIC 
    ${INCLUDE_DIR}
    ${CURSES_INCLUDE_DIRS}
    ${LUA_INCLUDE_DIR}
    ${Boost_INCLUDE_DIRS}
    ${MINIAUDIO}
)

# ---------------------------------- Libraries ----------------------------------

target_link_libraries(${EXECUTABLE_NAME} 
  ${LUA_LIBRARIES}
  ${CURSES_LIBRARIES}
  ${Boost_LIBRARIES}
  panel
)