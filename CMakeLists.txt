cmake_minimum_required(VERSION 3.0.0)
project(MyuEngine VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(${PROJECT_NAME} 
    src/main.cpp

    # glad
    src/glad.c

    # imgui
    vendor/imgui/imgui_demo.cpp
    vendor/imgui/imgui_draw.cpp
    vendor/imgui/imgui_tables.cpp
    vendor/imgui/imgui_widgets.cpp
    vendor/imgui/imgui.cpp
    vendor/imgui/backends/imgui_impl_glfw.cpp
    vendor/imgui/backends/imgui_impl_opengl3.cpp 
    vendor/ImGuiFileDialog/ImGuiFileDialog.cpp

    src/APIClient.h 
    src/APIClient.cpp
    src/SDAPIs.hpp
    src/LLMAPIs.hpp
    src/ImageGeneratorWindow.h
    src/ImageGeneratorWindow.cpp
    src/NovelGeneratorWindow.h
    src/NovelGeneratorWindow.cpp

)


set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

include(CTest)
enable_testing()

include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/vendor/imgui
    ${PROJECT_SOURCE_DIR}/vendor/imgui/backends
)

# link glfw
add_subdirectory(${PROJECT_SOURCE_DIR}/vendor/glfw)
target_link_libraries(${PROJECT_NAME} glfw)

# link httplib
add_subdirectory(${PROJECT_SOURCE_DIR}/vendor/cpp-httplib)
target_link_libraries(${PROJECT_NAME} httplib::httplib)
