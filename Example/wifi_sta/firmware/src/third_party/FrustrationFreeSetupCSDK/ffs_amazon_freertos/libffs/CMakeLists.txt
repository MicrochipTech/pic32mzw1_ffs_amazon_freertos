cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

# Collect all source of compat implementation
file(GLOB_RECURSE FFS_AMAZON_FREE_RTOS_COMPAT_SOURCE
        src/*.c)

# Collect all the FFS sources
file(GLOB_RECURSE FFS_C_SDK_SOURCES FOLLOW_SYMLINKS
        ${FFSSDK_FOLDER}/libffs/src/*.c)

# Set espressif component build variables
set(COMPONENT_SRCS ${FFS_AMAZON_FREE_RTOS_COMPAT_SOURCE}
        ${FFS_C_SDK_SOURCES})
        
set(COMPONENT_ADD_INCLUDEDIRS include
        ${FFSSDK_FOLDER}/libffs/include)

# Register component to the build system
register_component()

# Link Amazon Free RTOS libraries
target_link_libraries(${COMPONENT_TARGET}
        PRIVATE
            AFR::common
            AFR::wifi
            AFR::kernel
            AFR::https
            AFR::utils
            AFR::pkcs11
            3rdparty::mbedtls)

# Compile time flags
target_compile_definitions(${COMPONENT_TARGET}
        PUBLIC -DFFS_DEBUG -DFFS_WIFI_PROVISIONEE_ENABLED=1)