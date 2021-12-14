cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

# Find folders
set(LIBFFS_FOLDER "libffs_release")
if(NOT EXISTS ${LIBFFS_FOLDER})
    set(LIBFFS_FOLDER "libffs")
endif()

set(FFSSDK_FOLDER ${CMAKE_SOURCE_DIR}/../FrustrationFreeSetupCSDK)
if(NOT EXISTS ${FFSSDK_FOLDER})
    set(FFSSDK_FOLDER ${CMAKE_SOURCE_DIR}/..)
endif()

set(APP_FOLDER espressif_app/wifi_provisionee)

# Check enviroment variables
if(NOT DEFINED ENV{IDF_PATH} OR NOT ENV{IDF_PATH} STREQUAL ${CMAKE_HOME_DIRECTORY}/freertos/vendors/espressif/esp-idf/)
    set(ENV{IDF_PATH} ${CMAKE_HOME_DIRECTORY}/freertos/vendors/espressif/esp-idf/)
endif()

# Download Amazon FreeRTOS
if(NOT EXISTS ${CMAKE_HOME_DIRECTORY}/freertos)
    message(INFO "Please download Amazon FreeRTOS to freertos folder. Use the command: git clone --branch 202002.00 https://github.com/aws/amazon-freertos.git --recurse-submodules freertos")
endif()

# Python environment
if(NOT EXISTS ${CMAKE_HOME_DIRECTORY}/python_environment)
    message(INFO "Setting python environment...")
    execute_process ( COMMAND python3.7 -m venv python_environment )
    execute_process ( COMMAND source ./python_environment/bin/activate )
    execute_process ( COMMAND pip install -r freertos/vendors/espressif/esp-idf/requirements.txt )
endif()

# Setup toolchain
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/freertos/vendors/espressif/esp-idf/tools/cmake/toolchain-esp32.cmake)
set(CMAKE_GENERATOR "Ninja")

project(FrustrationFreeSetupCEspressif)

# Set include files for the app
include_directories(BEFORE espressif_app/configs)

message(INFO "Creating build files for Wifi Provisionee Application")

# Specify platform. This macro is used in ffsDirectedScan. For the meaning of the values, refer to ${LIBFFS_FOLDER}/include/ffs_amazon_freertos_platforms.h
add_definitions( -DFFS_DIRECTED_SCANNING_PLATFORM=FFS_PLATFORM_ESP32 )

# Add wifi provisionee cmake subdirectory
add_subdirectory(${APP_FOLDER})

# Tell IDF build to link against this target.
set(IDF_PROJECT_EXECUTABLE FrustrationFreeSetupCWifiProvisionee)

# add app executable
add_executable(FrustrationFreeSetupCWifiProvisionee ${APP_SOURCES})

# Add FFS Amazon Free RTOS lib as a component
get_filename_component(
    EXTRA_COMPONENT_DIRS
        ${LIBFFS_FOLDER} ABSOLUTE
)
list(APPEND IDF_EXTRA_COMPONENT_DIRS ${EXTRA_COMPONENT_DIRS})

# Add FreeRTOS as a subdirectory. AFR_BOARD tells which board to target.
set(AFR_BOARD espressif.esp32_devkitc CACHE INTERNAL "")
add_subdirectory(${CMAKE_HOME_DIRECTORY}/freertos)

# link required libraries
target_link_libraries(FrustrationFreeSetupCWifiProvisionee 
    PRIVATE 
        AFR::common
        AFR::wifi
        AFR::kernel
        AFR::https
        AFR::utils
        AFR::pkcs11
        3rdparty::mbedtls)
