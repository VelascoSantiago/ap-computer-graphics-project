cmake_minimum_required(VERSION 3.10)
project(CGhci C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(assimp REQUIRED)

add_library(glad STATIC dependencias/glad/src/glad.c)
target_include_directories(glad PUBLIC dependencias/glad/include)

file(GLOB SOIL2_SRC dependencias/SOIL2/*.c)
add_library(soil2 STATIC ${SOIL2_SRC})
target_include_directories(soil2 PUBLIC dependencias/SOIL2)
target_link_libraries(soil2 PUBLIC OpenGL::GL)   # <- nueva

file(GLOB TODAS RELATIVE ${CMAKE_SOURCE_DIR} *)
foreach(carpeta ${TODAS})
    if(EXISTS ${CMAKE_SOURCE_DIR}/${carpeta}/CMakeLists.txt AND NOT carpeta STREQUAL "template")
        add_subdirectory(${carpeta})
    endif()
endforeach()