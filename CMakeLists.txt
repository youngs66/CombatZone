cmake_minimum_required(VERSION 3.10)
project(glb_viewer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find required packages first
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(GLEW REQUIRED)

# Create external directory if it doesn't exist
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/external)

# Clone GLM
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/external/glm/CMakeLists.txt")
    message(STATUS "Cloning GLM...")
    execute_process(
        COMMAND git clone --depth 1 https://github.com/g-truc/glm.git
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/external
    )
endif()

# Download TinyGLTF if not present
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/external/tinygltf/tiny_gltf.h")
    message(STATUS "Downloading TinyGLTF...")
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/external/tinygltf)
    file(DOWNLOAD
        "https://raw.githubusercontent.com/syoyo/tinygltf/master/tiny_gltf.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/external/tinygltf/tiny_gltf.h"
        SHOW_PROGRESS
    )
    file(DOWNLOAD
        "https://raw.githubusercontent.com/nlohmann/json/develop/single_include/nlohmann/json.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/external/tinygltf/json.hpp"
        SHOW_PROGRESS
    )
endif()

# Download STB if not present
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/external/stb/stb_image.h")
    message(STATUS "Downloading STB...")
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/external/stb)
    file(DOWNLOAD
        "https://raw.githubusercontent.com/nothings/stb/master/stb_image.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/external/stb/stb_image.h"
        SHOW_PROGRESS
    )
endif()

# Add source files
add_executable(${PROJECT_NAME}
    src/main.cpp
    src/shader.cpp
    src/model.cpp
    src/camera.cpp
    src/implementations.cpp
    src/entity.cpp
    src/scene.cpp
)

# Set include directories
target_include_directories(${PROJECT_NAME} 
    PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/external
    ${CMAKE_CURRENT_SOURCE_DIR}/external/glm
    ${CMAKE_CURRENT_SOURCE_DIR}/external/tinygltf
    ${CMAKE_CURRENT_SOURCE_DIR}/external/stb
    ${OPENGL_INCLUDE_DIR}
    ${GLEW_INCLUDE_DIRS}
    /opt/homebrew/include  # For Homebrew includes
)

# Update source file includes
file(GLOB_RECURSE SOURCE_FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h
)

foreach(SOURCE_FILE ${SOURCE_FILES})
    file(READ ${SOURCE_FILE} CONTENT)
    string(REPLACE "<glm/glm.hpp>" "\"../external/glm/glm/glm.hpp\"" CONTENT "${CONTENT}")
    string(REPLACE "<glm/gtc/matrix_transform.hpp>" "\"../external/glm/glm/gtc/matrix_transform.hpp\"" CONTENT "${CONTENT}")
    string(REPLACE "<tinygltf/tiny_gltf.h>" "\"../external/tinygltf/tiny_gltf.h\"" CONTENT "${CONTENT}")
    string(REPLACE "\"stb_image.h\"" "\"../external/stb/stb_image.h\"" CONTENT "${CONTENT}")
    file(WRITE ${SOURCE_FILE} "${CONTENT}")
endforeach()

if(APPLE)
    target_link_directories(${PROJECT_NAME}
        PRIVATE
        /opt/homebrew/lib
    )
endif()

# Link libraries
target_link_libraries(${PROJECT_NAME}
    PRIVATE
    OpenGL::GL
    GLEW::GLEW
    glfw
)

# Add compile definitions
if(USE_OPENGL_ES)
    target_compile_definitions(${PROJECT_NAME} PRIVATE USE_GLES2)
endif() 