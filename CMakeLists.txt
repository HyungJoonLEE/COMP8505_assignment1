cmake_minimum_required(VERSION 3.22)

# Project name and a few useful settings. Other commands can pick up the results
project(COMP8505_assignment1
        VERSION 0.1
        DESCRIPTION ""
        LANGUAGES C)

set(HEADER_LIST
        "${PROJECT_SOURCE_DIR}/include/common.h"
        "${PROJECT_SOURCE_DIR}/include/server.h"
        "${PROJECT_SOURCE_DIR}/include/server_helper.h"
        "${PROJECT_SOURCE_DIR}/include/client.h"
        "${PROJECT_SOURCE_DIR}/include/client_helper.h"
        "${PROJECT_SOURCE_DIR}/include/error.h"
        "${PROJECT_SOURCE_DIR}/include/conversion.h"
        )

set(COMMON_SOURCE_LIST
        "${PROJECT_SOURCE_DIR}/src/common.c"
        "${PROJECT_SOURCE_DIR}/src/error.c"
        "${PROJECT_SOURCE_DIR}/src/conversion.c"
        )

set(SERVER_SOURCE_LIST
        "${PROJECT_SOURCE_DIR}/src/server.c"
        "${PROJECT_SOURCE_DIR}/src/server_helper.c"
        )

set(CLIENT_SOURCE_LIST
        "${PROJECT_SOURCE_DIR}/src/client.c"
        "${PROJECT_SOURCE_DIR}/src/client_helper.c"
        )

### Require out-of-source builds
# this still creates a CMakeFiles directory and CMakeCache.txt- can we delete them?
file(TO_CMAKE_PATH "${PROJECT_BINARY_DIR}/CMakeLists.txt" LOC_PATH)
if (EXISTS "${LOC_PATH}")
    message(FATAL_ERROR "You cannot build in a source directory (or any directory with a CMakeLists.txt file). Please make a build subdirectory. Feel free to remove CMakeCache.txt and CMakeFiles.")
endif ()

# Only do these if this is the main project, and not if it is included through add_subdirectory
if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)

    # Optionally set things like CMAKE_CXX_STANDARD, CMAKE_POSITION_INDEPENDENT_CODE here
    set(CMAKE_C_STANDARD 17)
    set(CMAKE_C_EXTENSIONS OFF)

    # Let's nicely support folders in IDEs
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)

    # Testing only available if this is the main app
    # Note this needs to be done in the main CMakeLists
    # since it calls enable_testing, which must be in the
    # main CMakeLists.

endif ()

# The compiled library code is here
add_subdirectory(src)

find_library(LIBCGREEN cgreen)