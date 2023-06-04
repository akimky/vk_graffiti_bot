cmake_minimum_required(VERSION 3.11)
project(vk_graffiti_bot)
set(CMAKE_CXX_STANDARD 17)

find_package(CURL REQUIRED)
find_package(SFML 2.5 COMPONENTS graphics REQUIRED)

file(GLOB_RECURSE SOURCES sources/*.cpp)
include_directories(include third_party/include)
add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} ${CURL_LIBRARIES} sfml-graphics)