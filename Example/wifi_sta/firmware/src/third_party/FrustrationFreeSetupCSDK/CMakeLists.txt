cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)
project(FrustrationFreeSetup)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_VERBOSE_MAKEFILE ON)

# 0 warnings.
set_property(DIRECTORY . APPEND PROPERTY COMPILE_OPTIONS -Wall -Wextra -Werror)

file(GLOB_RECURSE FFS_COMMON_SOURCES
    libffs/src/*.c
    )

add_library(FrustrationFreeSetup STATIC
    ${FFS_COMMON_SOURCES}
    )

target_include_directories(FrustrationFreeSetup PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/libffs/include>
    $<INSTALL_INTERFACE:include>
    )

# Debug.
option(ENABLE_DEBUG "Enable debug" ON)
if (${ENABLE_DEBUG})
    message("FFS - Enable debug")

    target_compile_definitions(FrustrationFreeSetup PUBLIC
        -DFFS_DEBUG
        )
endif()

# Testing
option(ENABLE_TESTS "Enable tests" ON)
if (${ENABLE_TESTS})
    message("FFS - Enable tests")

    if(NOT DEFINED BRAZIL_TESTING)
        set(BRAZIL_TESTING ON)
    endif()

    enable_testing()

    include(libffs/test/LocalCoverage.cmake)
    add_subdirectory(libffs/test)
endif()

if(NOT DEFINED ${CMAKE_INSTALL_LIBDIR})
    set(CMAKE_INSTALL_LIBDIR ${CMAKE_INSTALL_PREFIX}/lib)
endif()

export(TARGETS FrustrationFreeSetup FILE FrustrationFreeSetupConfig.cmake)
install(TARGETS FrustrationFreeSetup EXPORT FrustrationFreeSetupConfig
    ARCHIVE  DESTINATION lib
    LIBRARY  DESTINATION lib
    RUNTIME  DESTINATION bin)  # This is for Windows
install(EXPORT FrustrationFreeSetupConfig DESTINATION cmake)
install(DIRECTORY libffs/include/ DESTINATION include)
