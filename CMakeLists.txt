cmake_minimum_required (VERSION 3.10)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 26)
cmake_policy(SET CMP0167 OLD)

set(CMAKE_COMPILE_WARNING_AS_ERROR ON)

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
set(CSV_PARSER "${EXTERN_DIR}/fast-cpp-csv-parser")

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

# set(CURSES_NEED_NCURSES true)
# set(CURSES_NEED_WIDE true)

find_package(Curses REQUIRED)
if (NOT Curses_FOUND)
  message(FATAL_ERROR "Could not find a supported ncurses library. Do you have ncurses installed on your system?")
elseif(NOT TARGET Curses::Curses)
  add_library(Curses::Curses INTERFACE IMPORTED)
  set_target_properties(
    Curses::Curses
    PROPERTIES
      INTERFACE_LINK_LIBRARIES "${CURSES_LIBRARIES}"
      INTERFACE_INCLUDE_DIRECTORIES "${CURSES_INCLUDE_DIRS}"
  )
endif()
if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
  add_compile_definitions(-DNCURSES_STATIC)
endif()

find_package(Lua REQUIRED)
if (NOT Lua_FOUND)
  message(FATAL_ERROR "Could not find a supported lua c library. Do you have lua installed on your system?")
elseif(Lua_FOUND AND NOT TARGET Lua::Lua)
  add_library(Lua::Lua INTERFACE IMPORTED)
  set_target_properties(
    Lua::Lua
    PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES "${LUA_INCLUDE_DIR}"
      INTERFACE_LINK_LIBRARIES "${LUA_LIBRARIES}"
  )
endif()

# ----------------------------------- Add EXE -----------------------------------

add_executable(${EXECUTABLE_NAME} ${SOURCES} "${MINIAUDIO}/miniaudio.c")

if ((DEFINED NO_RUNTIME_DIRECTORY) AND (NO_RUNTIME_DIRECTORY EQUAL true))

elseif(DEFINED RUNTIME_DIRECTORY)
  target_compile_definitions(${EXECUTABLE_NAME} PUBLIC SOURCE_RUNTIME_DIRECTORY="${RUNTIME_DIRECTORY}/")
else()
  target_compile_definitions(${EXECUTABLE_NAME} PUBLIC SOURCE_RUNTIME_DIRECTORY="${CMAKE_CURRENT_SOURCE_DIR}/runtime/")
endif()

# ----------------------------- Include Directories -----------------------------

target_include_directories(${EXECUTABLE_NAME} PUBLIC 
    "${INCLUDE_DIR}"
    "${MINIAUDIO}"
    "${CSV_PARSER}"
)

# ---------------------------------- Libraries ----------------------------------
target_link_libraries(${EXECUTABLE_NAME} 
  Lua::Lua
  Curses::Curses
  panelw
)