cmake_minimum_required(VERSION 3.22)

set(PROJECT_NAME kortex_driver)
set(PROJECT_VERSION 1.0.0)
project(
  ${PROJECT_NAME}
  LANGUAGES CXX
  VERSION ${PROJECT_VERSION}
)

set(CXX_DISABLE_WERROR 1)
set(CMAKE_CXX_STANDARD 17)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)
include(GNUInstallDirs)

# option(BUILD_TESTING "Build tests" ON)

# Dependencies -------------------------------------------------------------------------------------
find_package(mc_robot_interface REQUIRED)
find_package(fmt REQUIRED)
find_package(Threads REQUIRED)

# Kortex search ------------------------------------------------------------------------------------
set(KORTEX_API_VERSION
    "2.8.0"
    CACHE STRING "Kortex API version"
)

set(KORTEX_ROOT_DIR
    ""
    CACHE PATH "Path to an existing Kortex API installation"
)

set(KORTEX_API_URL
    "https://artifactory.kinovaapps.com/artifactory/generic-public/kortex/API/${KORTEX_API_VERSION}/linux_x86-64_gcc_5.4.zip"
)

find_path(
  KORTEX_API_INCLUDE_DIR
  NAMES ActuatorConfigClientRpc.h
  HINTS "${KORTEX_ROOT_DIR}"
  PATH_SUFFIXES include include/client include/client_stubs
)

find_library(
  KORTEX_API_LIBRARY
  NAMES KortexApiCpp libKortexApiCpp
  HINTS "${KORTEX_ROOT_DIR}"
  PATH_SUFFIXES lib lib/release
)

if(KORTEX_API_INCLUDE_DIR AND KORTEX_API_LIBRARY)
  if(NOT KORTEX_ROOT_DIR)
    # Reconstruct:
    # <root>/lib/release/libKortexApiCpp.a
    get_filename_component(
      KORTEX_API_RELEASE_DIR "${KORTEX_API_LIBRARY}" DIRECTORY
    )
    get_filename_component(
      KORTEX_API_LIB_DIR "${KORTEX_API_RELEASE_DIR}" DIRECTORY
    )
    get_filename_component(KORTEX_ROOT_DIR "${KORTEX_API_LIB_DIR}" DIRECTORY)
  endif()

  message(STATUS "")
  message(STATUS "========== Existing Kortex API ==========")
  message(STATUS "Root: ${KORTEX_ROOT_DIR}")
  message(STATUS "Headers: ${KORTEX_API_INCLUDE_DIR}")
  message(STATUS "Library: ${KORTEX_API_LIBRARY}")
  message(STATUS "Fetching: no")
  message(STATUS "=========================================")
  message(STATUS "")
else()
  # FetchContent if not found ------------------------------------------------------------------------
  if(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux" OR NOT CMAKE_SYSTEM_PROCESSOR
                                               STREQUAL "x86_64"
  )
    message(
      FATAL_ERROR
        "No existing Kortex API installation was found and automatic "
        "download currently supports Linux x86_64 only. "
        "Set KORTEX_ROOT_DIR manually."
    )
  endif()

  message(STATUS "")
  message(STATUS "========== Fetching Kortex API ==========")
  message(STATUS "Version: ${KORTEX_API_VERSION}")
  message(STATUS "Archive: ${KORTEX_API_URL}")
  message(STATUS "Destination: ${FETCHCONTENT_BASE_DIR}/kinova_binary_api-src")
  message(STATUS "Reason: existing Kortex API was not found")
  message(STATUS "=========================================")
  message(STATUS "")

  FetchContent_Declare(
    kinova_binary_api
    URL "${KORTEX_API_URL}"
    URL_HASH
      MD5=b9b68885df8e1dd0e5a2ba7285f14c0d # From https://github.com/Kinovarobotics/ros2_kortex/blob/main/kortex_api/CMakeLists.txt
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
  )

  FetchContent_MakeAvailable(kinova_binary_api)

  set(KORTEX_ROOT_DIR "${kinova_binary_api_SOURCE_DIR}")

  find_path(
    KORTEX_API_INCLUDE_DIR
    NAMES ActuatorConfigClientRpc.h REQUIRED
    HINTS "${KORTEX_ROOT_DIR}"
    PATH_SUFFIXES include include/client include/client_stubs
    NO_DEFAULT_PATH
  )

  find_library(
    KORTEX_API_LIBRARY
    NAMES KortexApiCpp libKortexApiCpp REQUIRED
    HINTS "${KORTEX_ROOT_DIR}"
    PATH_SUFFIXES lib/release
    NO_DEFAULT_PATH
  )

  message(STATUS "")
  message(STATUS "========== Kortex API ready ==========")
  message(STATUS "Extracted root: ${KORTEX_ROOT_DIR}")
  message(STATUS "Headers: ${KORTEX_API_INCLUDE_DIR}")
  message(STATUS "Library: ${KORTEX_API_LIBRARY}")
  message(STATUS "======================================")
  message(STATUS "")
endif()

# Import Kortex ------------------------------------------------------------------------------------
set(KORTEX_API_INCLUDE_DIRECTORIES
    "${KORTEX_ROOT_DIR}/include"
    "${KORTEX_ROOT_DIR}/include/client"
    "${KORTEX_ROOT_DIR}/include/common"
    "${KORTEX_ROOT_DIR}/include/messages"
    "${KORTEX_ROOT_DIR}/include/client_stubs"
)

foreach(KORTEX_INCLUDE_DIRECTORY IN LISTS KORTEX_API_INCLUDE_DIRECTORIES)
  if(NOT EXISTS "${KORTEX_INCLUDE_DIRECTORY}")
    message(FATAL_ERROR "Kortex include directory was not found: "
                        "${KORTEX_INCLUDE_DIRECTORY}"
    )
  endif()
endforeach()

add_library(KortexApiCpp STATIC IMPORTED GLOBAL)

set_target_properties(
  KortexApiCpp
  PROPERTIES IMPORTED_LOCATION "${KORTEX_API_LIBRARY}"
             INTERFACE_INCLUDE_DIRECTORIES "${KORTEX_API_INCLUDE_DIRECTORIES}"
             INTERFACE_COMPILE_DEFINITIONS "_OS_UNIX"
)

add_library(Kortex::Api ALIAS KortexApiCpp)

target_link_libraries(KortexApiCpp INTERFACE Threads::Threads ${CMAKE_DL_LIBS})

# --------------------------------------------------------------------------------------------------

add_subdirectory(src)

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# if(BUILD_TESTING)
#   enable_testing()
#   add_subdirectory(tests)
# endif()
