cmake_minimum_required(VERSION 3.22)
project(HelloWorld VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)
find_package(glfw3 3.3 REQUIRED)

add_executable(hello_world src/main.cpp src/glad.c src/mesh.cpp src/shader.cpp)

target_include_directories(hello_world PRIVATE dependencies)
target_include_directories(hello_world PRIVATE /opt/homebrew/include)

target_link_libraries(hello_world glfw OpenGL::GL)