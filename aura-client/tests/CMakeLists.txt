cmake_minimum_required(VERSION 3.0)
enable_testing()

set(LINUX_LIBS curl)
set(WIN32_LIBS urlmon wininet)

# Set include directories for needed header files
set(CATCH2_INCLUDE "${PROJECT_SOURCE_DIR}/deps/Catch2/single_include/")
set(PICOSHA2_INCLUDE "${PROJECT_SOURCE_DIR}/deps/PicoSHA2/")
set(RAPIDJSON_INCLUDE "${PROJECT_SOURCE_DIR}/deps/rapidjso/include/")
include_directories(
    ${PROJECT_SOURCE_DIR}
    ${CATCH2_INCLUDE}
    ${PICOSHA2_INCLUDE}
    ${RAPIDJSON_INCLUDE}
)

# Set Catch2 main file as library
set(TEST_MAIN tests-main)

if(UNIX AND NOT APPLE)
    set(CATCH2_MAIN_LIB_TYPE SHARED)
endif()

if(WIN32)
    set(CATCH2_MAIN_LIB_TYPE STATIC)
endif()

add_library(${TEST_MAIN} ${CATCH2_MAIN_LIB_TYPE} ${TEST_MAIN}.cc)

# Set all test source files, compile, and link to main test file
# Then link libraries for OS
# Finally, add test to CMake project
add_executable(tests-authfile tests-authfile.cc "${PROJECT_SOURCE_DIR}/authfile.cc")
add_executable(tests-bot tests-bot.cc
    "${PROJECT_SOURCE_DIR}/bot.cc"
    "${PROJECT_SOURCE_DIR}/request.cc"
    "${PROJECT_SOURCE_DIR}/util.cc"
    "${PROJECT_SOURCE_DIR}/sysinfo.cc"
    "${PROJECT_SOURCE_DIR}/authfile.cc"
    "${PROJECT_SOURCE_DIR}/installer.cc")
add_executable(tests-installer tests-installer.cc
    "${PROJECT_SOURCE_DIR}/authfile.cc"
    "${PROJECT_SOURCE_DIR}/installer.cc"
    "${PROJECT_SOURCE_DIR}/util.cc")
add_executable(tests-request tests-request.cc "${PROJECT_SOURCE_DIR}/request.cc")
add_executable(tests-util tests-util.cc "${PROJECT_SOURCE_DIR}/util.cc")

set(TESTS
    tests-authfile
    tests-bot
    tests-installer
    tests-request
    tests-util
)

foreach(TEST_SRC_FILE ${TESTS})
    target_link_libraries(${TEST_SRC_FILE} ${TEST_MAIN})

    if(UNIX AND NOT APPLE)
        target_link_libraries(${TEST_SRC_FILE} ${LINUX_LIBS})
    endif()
    if(WIN32)
        target_link_libraries(${TEST_SRC_FILE} ${WIN32_LIBS})
    endif()

    add_test(${TEST_SRC_FILE} ${TEST_SRC_FILE})
endforeach(TEST_SRC_FILE)
