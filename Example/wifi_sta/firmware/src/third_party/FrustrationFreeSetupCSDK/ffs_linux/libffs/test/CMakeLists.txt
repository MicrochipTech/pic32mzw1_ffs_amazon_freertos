find_package(GTest ${REQUIRED_WHEN_TESTING})

if(BRAZIL_BUILD)
    find_package(GMock ${REQUIRED_WHEN_TESTING})
else()
    set(GMOCK_INCLUDE_DIR /usr/local/include/gmock)
    set(GMOCK_LIBRARY /usr/local/lib/libgmock.a)
endif()

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${CMAKE_CURRENT_SOURCE_DIR}/helpers
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${GTEST_INCLUDE_DIR}
    ${GMOCK_INCLUDE_DIR}
    )

file(GLOB_RECURSE TEST_SOURCES *.cpp sha-256.c)

add_executable(all_tests
    ${TEST_SOURCES}
    )

if(BRAZIL_BUILD)
    target_compile_definitions(all_tests PUBLIC
        -DBRAZIL
        )
endif()

if(APPLE)
target_link_libraries(all_tests
    FrustrationFreeSetup
    FrustrationFreeSetupLinux
    ${GTEST_LIBRARY}
    ${GTEST_MAIN_LIBRARY}
    ${GMOCK_LIBRARY}
    ${CURL_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    OpenSSL::Crypto
    )
else()
target_link_libraries(all_tests
    -Wl,--start-group
    FrustrationFreeSetup
    FrustrationFreeSetupLinux
    -Wl,--end-group
    ${GTEST_LIBRARY}
    ${GTEST_MAIN_LIBRARY}
    ${GMOCK_LIBRARY}
    ${CURL_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    OpenSSL::Crypto
    )
endif()

add_test(NAME all_tests COMMAND all_tests --gtest_output=xml:${BRAZIL_TEST_OUTPUT}/TEST-FrustrationFreeSetupLinux.xml)

add_local_coverage_target(all_tests)
