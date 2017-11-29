cmake_minimum_required(VERSION 3.9)
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
add_library(${TEST_MAIN} SHARED ${TEST_MAIN}.cc)

# Set all test source files, compile, and link to main test file
# Then link libraries for OS
# Finally, add test to CMake project
set(TESTS
    tests-request
    tests-system
)

foreach(TEST_SRC_FILE ${TESTS})
    add_executable(${TEST_SRC_FILE} ${TEST_SRC_FILE}.cc)
    target_link_libraries(${TEST_SRC_FILE} ${TEST_MAIN})

    if(UNIX AND NOT APPLE)
        target_link_libraries(${TEST_SRC_FILE} ${LINUX_LIBS})
    endif()
    if(WIN32)
        target_link_libraries(${TEST_SRC_FILE} ${WIN32_LIBS})
    endif()

    add_test(${TEST_SRC_FILE} ${TEST_SRC_FILE})
endforeach(TEST_SRC_FILE)

