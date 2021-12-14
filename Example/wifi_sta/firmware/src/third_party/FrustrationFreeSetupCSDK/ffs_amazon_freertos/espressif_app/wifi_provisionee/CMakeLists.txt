cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

project(FrustrationFreeSetupCEspressifWifiProvisionee)

# Create a list of source files
file(GLOB_RECURSE APP_SOURCES 
        src/*.c 
        *.c)

# App includes
include_directories(include)

set(APP_SOURCES ${APP_SOURCES} PARENT_SCOPE)
