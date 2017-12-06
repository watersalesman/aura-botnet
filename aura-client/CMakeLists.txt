cmake_minimum_required(VERSION 3.0)
project (Aura)
add_definitions(-DAURA_VERSION="0.1.2")

set(BIN aura)

# Set default CMake build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif(NOT CMAKE_BUILD_TYPE)

# Add include directories
set(PICOSHA2_INCLUDE "${CMAKE_SOURCE_DIR}/deps/PicoSHA2/")
set(RAPIDJSON_INCLUDE "${CMAKE_SOURCE_DIR}/deps/rapidjson/include/")
include_directories(${PICOSHA2_INCLUDE} ${RAPIDJSON_INCLUDE})

# Set libraries
set(LINUX_LIBS curl)
set(WIN32_LIBS urlmon wininet)

if(UNIX AND NOT APPLE)
    add_definitions(-DCURL_STATICLIB)
    add_executable(${BIN} aura.cc aura.hh)
    target_link_libraries(${BIN} ${LINUX_LIBS})
endif()

if(WIN32)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_executable(${BIN} aura.cc aura.hh)
    target_link_libraries(${BIN} ${WIN32_LIBS})
endif()

# Define BUILD_TESTING to build tests
if(CMAKE_BUILD_TYPE MATCHES Debug)
    enable_testing()
    add_subdirectory(tests)
endif()
