cmake_minimum_required (VERSION 3.10)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_COMPILER )

set(PROJECT_NAME ProjectNAME)
set(VERSION "0.0.1")

set(EXECUTABLE_NAME ExeNAME)

project(${PROJECT_NAME} VERSION ${VERSION} LANGUAGES C CXX)

# -------------------------------- Directories --------------------------------

set(LIBRARY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/bin/${CMAKE_SYSTEM_NAME}/${PRESET_NAME}")

set(STATIC_DIR "${LIBRARY_DIR}/lib")
set(SHARED_DIR "${LIBRARY_DIR}/dyn")
set(EXPORT_DIR "${LIBRARY_DIR}/export")
set(INSTALL_DIR "${LIBRARY_DIR}/install")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${STATIC_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${SHARED_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${SHARED_DIR})

# --------------------------------- Set Sources ---------------------------------
set(SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")

FILE(GLOB SOURCES "${SOURCE_DIR}/*.cpp")

# ------------------------------- Platform Guards -------------------------------

if ((NOT (( CMAKE_SYSTEM_NAME STREQUAL "Darwin") OR (CMAKE_SYSTEM_NAME STREQUAL "Windows"))) OR (NOT CMAKE_SIZEOF_VOID_P EQUAL 8))
  message(FATAL_ERROR "${PROJECT_NAME} only supports 64-bit MacOS and Windows systems!")
endif()

# ----------------------------------- Add EXE -----------------------------------

add_executable(${EXECUTABLE_NAME} ${SOURCES})

target_include_directories(${EXECUTABLE_NAME} PUBLIC ${INCLUDE_DIR})