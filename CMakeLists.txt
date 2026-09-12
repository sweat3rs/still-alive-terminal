cmake_minimum_required (VERSION 3.10)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 26)

set(PROJECT_NAME still-alive-terminal)
set(VERSION "0.0.1")

set(EXECUTABLE_NAME still_alive)

project(${PROJECT_NAME} VERSION ${VERSION} LANGUAGES C CXX)

# -------------------------------- Directories --------------------------------

set(EXTERN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/extern")

set(LIBRARY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/bin/${CMAKE_SYSTEM_NAME}/${PRESET_NAME}")

set(STATIC_DIR "${LIBRARY_DIR}/lib")
set(SHARED_DIR "${LIBRARY_DIR}/dyn")
set(EXPORT_DIR "${LIBRARY_DIR}/export")
set(INSTALL_DIR "${LIBRARY_DIR}/install")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${STATIC_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${SHARED_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${SHARED_DIR})

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
set(CURSES_NEED_WIDE true)

find_package(Curses REQUIRED)

if (NOT Curses_FOUND)
  message(FATAL_ERROR "Could not find a supported ncurses library. Do you have ncurses installed on your system?")
endif()

if(Curses_FOUND AND NOT TARGET Curses::Curses)
  add_library(Curses::Curses INTERFACE IMPORTED)
  set_target_properties(
    Curses::Curses
    PROPERTIES
      INTERFACE_LINK_LIBRARIES "${CURSES_LIBRARIES}"
      INTERFACE_INCLUDE_DIRECTORIES "${CURSES_INCLUDE_DIRS}"
  )
elseif(NOT Curses_FOUND)
  message(FATAL_ERROR "Could not find a supported ncurses library. Do you have ncurses installed on your system?")
endif()

find_package(Lua REQUIRED)

if (NOT Lua_FOUND)
  message(FATAL_ERROR "Could not find a supported lua c library. Do you have lua installed on your system?")
endif()

# ----------------------------------- Add EXE -----------------------------------

add_executable(${EXECUTABLE_NAME} ${SOURCES} "${MINIAUDIO}/miniaudio.c")

# ----------------------------- Include Directories -----------------------------

target_include_directories(${EXECUTABLE_NAME} PUBLIC 
    ${INCLUDE_DIR}
    ${CURSES_INCLUDE_DIR}
    ${LUA_INCLUDE_DIR}
    ${MINIAUDIO}
)

add_compile_definitions(SOURCE_RUNTIME_DIRECTORY="${CMAKE_CURRENT_SOURCE_DIR}/runtime/")

# ---------------------------------- Libraries ----------------------------------

target_link_libraries(${EXECUTABLE_NAME} 
  ${CURSES_LIBRARIES}
  Curses::Curses
  panel
  ${LUA_LIBRARIES})