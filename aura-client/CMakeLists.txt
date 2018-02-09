cmake_minimum_required(VERSION 3.0)
project (aura-client)


add_definitions(-DAURA_VERSION="0.1.2")

# Set default CMake build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif(NOT CMAKE_BUILD_TYPE)

# Add include directories
set(PICOSHA2_INCLUDE "${CMAKE_SOURCE_DIR}/deps/PicoSHA2/")
set(RAPIDJSON_INCLUDE "${CMAKE_SOURCE_DIR}/deps/rapidjson/include/")
include_directories(${PICOSHA2_INCLUDE} ${RAPIDJSON_INCLUDE})

# Set libraries
set(LINUX_LIBS curl stdc++fs pthread)
set(WIN32_LIBS urlmon wininet)

# Set standard
set (CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set executable
add_executable(${PROJECT_NAME}
    aura.cc
    request.cc
    util.cc
    bot.cc
    installer.cc
    authfile.cc
    sysinfo.cc
)

if(UNIX AND NOT APPLE)
    add_definitions(-DCURL_STATICLIB)
    target_link_libraries(${PROJECT_NAME} ${LINUX_LIBS})
endif()

if(WIN32)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    target_link_libraries(${PROJECT_NAME} ${WIN32_LIBS})
endif()

# Define BUILD_TESTING to build tests
if(CMAKE_BUILD_TYPE MATCHES Debug)
    enable_testing()
    add_subdirectory(tests)
endif()
