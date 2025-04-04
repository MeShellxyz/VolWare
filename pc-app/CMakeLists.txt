cmake_minimum_required(VERSION 3.5)
project(VolFlex VERSION 0.1.0)

# set appropriate encoding
if (WIN32)
    add_compile_options(/utf-8)
elseif(APPLE)
    add_compile_options(-finput-charset=UTF-8)
elseif(UNIX)
    add_compile_options(-finput-charset=UTF-8)
endif()

include(FetchContent)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(WIN32)
    add_compile_definitions(WINDOWS)
    add_compile_definitions(_WIN32_WINNT=0x0601)
elseif(APPLE)
    add_compile_definitions(MACOS)
elseif(UNIX)
    add_compile_definitions(LINUX)
endif()

# Add source files
set(SOURCE_FILES
    src/main.cpp
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
