cmake_minimum_required(VERSION 2.8...3.13)

# Homebrew 경로 설정
if(APPLE)
    if(CMAKE_APPLE_SILICON_PROCESSOR)
        # Apple Silicon (M1/M2) Mac의 경우
        list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew")
    else()
        # Intel Mac의 경우
        list(APPEND CMAKE_PREFIX_PATH "/usr/local")
    endif()

    # pkg-config 경로 설정
    set(ENV{PKG_CONFIG_PATH} "/opt/homebrew/lib/pkgconfig:/usr/local/lib/pkgconfig:$ENV{PKG_CONFIG_PATH}")
endif()

project(MyuEngine VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(${PROJECT_NAME}
    src/main.cpp

    # stb_image
    vendor/stb/stb_image.cpp

    # imgui
    vendor/imgui/imgui_demo.cpp
    vendor/imgui/imgui_draw.cpp
    vendor/imgui/imgui_tables.cpp
    vendor/imgui/imgui_widgets.cpp
    vendor/imgui/imgui.cpp
    vendor/imgui/backends/imgui_impl_glfw.cpp
    vendor/imgui/backends/imgui_impl_opengl3.cpp

    # openGL base classes
    src/GL/glad.c
    src/GL/Texture2D.cpp
    src/GL/Shader.cpp
    src/GL/Sprite.cpp
    src/GL/Camera2D.cpp
    src/GL/VertexArrayObject.cpp
    src/GL/VertexBufferObject.cpp
    src/GL/IndexBufferObject.cpp
    src/GL/UIImage.cpp
    src/GL/UIElement.cpp
    src/GL/UIText.cpp

    # MyuEngine
    src/MyuEngine/Character.cpp
    src/MyuEngine/ShaderManager.cpp
    src/MyuEngine/CameraService.cpp
    src/MyuEngine/Background.cpp
    src/MyuEngine/ResourceManager.cpp
    src/MyuEngine/SpriteManager.cpp
    src/MyuEngine/ScriptManager.cpp
    src/MyuEngine/DialogueManager.cpp
)


set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

include(CTest)
enable_testing()

include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/vendor/imgui
    ${PROJECT_SOURCE_DIR}/vendor/imgui/backends
    ${PROJECT_SOURCE_DIR}/vendor/stb
)

# glfw 링크
add_subdirectory(${PROJECT_SOURCE_DIR}/vendor/glfw)
target_link_libraries(${PROJECT_NAME} PRIVATE glfw)

# zlib 찾기 (여기에 추가)
find_package(ZLIB REQUIRED)
if(ZLIB_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE ZLIB::ZLIB)
endif()

# freetype 링크
add_subdirectory(${PROJECT_SOURCE_DIR}/vendor/freetype)
target_link_libraries(${PROJECT_NAME} PRIVATE freetype)

# sol2 링크
add_subdirectory(${PROJECT_SOURCE_DIR}/vendor/sol2)
target_link_libraries(${PROJECT_NAME} PRIVATE sol2)

# lua 링크
add_subdirectory(${PROJECT_SOURCE_DIR}/vendor/lua-5.4.7)
target_link_libraries(${PROJECT_NAME} PRIVATE lua)