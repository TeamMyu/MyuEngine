cmake_minimum_required(VERSION 3.0.0)
project(MyuEngine VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(MyuEngine src/main.cpp src/glad.c)
set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

include(CTest)
enable_testing()

include_directories(
    ${PROJECT_SOURCE_DIR}/include
)

if (APPLE)
    target_link_libraries(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/lib/libglfw.3.dylib)
endif()

