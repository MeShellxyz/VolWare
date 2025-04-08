cmake_minimum_required(VERSION 3.5)
project(VolFlex VERSION 0.1.0)

# Define Windows API target version to avoid the warning
if(MSVC)
    add_definitions(-D_WIN32_WINNT=0x0601) # Windows 7
    add_compile_options(/utf-8)
endif()

include(FetchContent)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add source files
set(SOURCE_FILES
    src/main.cpp
    src/Config.cpp
    src/SerialReader.cpp
    src/VolumeController.cpp
)

FetchContent_Declare(
    yaml_cpp
    GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
    GIT_TAG        master
)
FetchContent_MakeAvailable(yaml_cpp)

FetchContent_Declare(
    boost
    GIT_REPOSITORY https://github.com/boostorg/boost.git
    GIT_TAG        boost-1.83.0
)
set(BOOST_INCLUDE_LIBRARIES asio system)
set(BOOST_ENABLE_CMAKE ON)
FetchContent_MakeAvailable(boost)

# Create the executable
add_executable(${PROJECT_NAME} ${SOURCE_FILES})

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(${PROJECT_NAME} PRIVATE yaml-cpp Boost::system Boost::asio)