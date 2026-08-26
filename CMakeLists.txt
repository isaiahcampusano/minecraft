cmake_minimum_required(VERSION 3.20)
project(MinecraftSuperflat VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_Declare(glfw GIT_REPOSITORY https://github.com/glfw/glfw.git GIT_TAG 3.4)
FetchContent_Declare(glm GIT_REPOSITORY https://github.com/g-truc/glm.git GIT_TAG 1.0.1)
FetchContent_Declare(glad GIT_REPOSITORY https://github.com/Dav1dde/glad.git GIT_TAG v2.0.8 SOURCE_SUBDIR cmake)
FetchContent_MakeAvailable(glfw glm glad)
glad_add_library(glad_gl_core_33 REPRODUCIBLE API gl:core=3.3)

add_executable(Minecraft
  src/main.cpp
  src/core/Application.cpp src/core/Camera.cpp src/core/Shader.cpp
  src/player/Player.cpp src/player/PlayerPhysics.cpp src/player/PlayerCamera.cpp
  src/renderer/Mesh.cpp src/renderer/Renderer.cpp src/renderer/Texture.cpp src/renderer/PlayerRenderer.cpp
  src/world/Chunk.cpp src/world/World.cpp src/world/WorldGenerator.cpp
  src/utils/Raycaster.cpp)
target_include_directories(Minecraft PRIVATE src)
target_link_libraries(Minecraft PRIVATE glfw glad_gl_core_33 glm::glm)

if(MSVC)
  target_compile_options(Minecraft PRIVATE /W4 /permissive-)
else()
  target_compile_options(Minecraft PRIVATE -Wall -Wextra -Wpedantic)
endif()

include(CTest)
if(BUILD_TESTING)
  add_executable(PlayerPhysicsTests
    tests/PlayerPhysicsTests.cpp
    src/player/Player.cpp src/player/PlayerPhysics.cpp
    src/world/World.cpp src/world/WorldGenerator.cpp src/world/Chunk.cpp
    src/renderer/Mesh.cpp src/core/Shader.cpp)
  target_include_directories(PlayerPhysicsTests PRIVATE src)
  target_link_libraries(PlayerPhysicsTests PRIVATE glad_gl_core_33 glm::glm)
  add_test(NAME player_physics COMMAND PlayerPhysicsTests)
endif()
