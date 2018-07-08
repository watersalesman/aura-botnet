cmake_minimum_required(VERSION 3.0)
enable_testing()

set(LINUX_LIBS curl stdc++fs pthread)
set(WIN32_LIBS urlmon wininet)

# Download Catch2
if(NOT EXISTS "${DEPS_FOLDER}/catch.hpp")
    file(DOWNLOAD "https://github.com/catchorg/Catch2/releases/download/v2.2.3/catch.hpp" "${DEPS_FOLDER}/catch.hpp" SHOW_PROGRESS)
endif(NOT EXISTS "${DEPS_FOLDER}/catch.hpp")

include_directories(
    ${PROJECT_SOURCE_DIR}
)

# Set standard
set (CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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

set(TESTS
    tests-authfile
    tests-bot
    tests-installer
    tests-request
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
