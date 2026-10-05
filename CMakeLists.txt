cmake_minimum_required(VERSION 3.20)
project(MinecraftSuperflat VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
  message(STATUS "CMAKE_BUILD_TYPE not set; defaulting to Release")
endif()

include(FetchContent)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_Declare(glfw GIT_REPOSITORY https://github.com/glfw/glfw.git GIT_TAG 3.4)
FetchContent_Declare(glm GIT_REPOSITORY https://github.com/g-truc/glm.git GIT_TAG 1.0.1)
FetchContent_Declare(glad GIT_REPOSITORY https://github.com/Dav1dde/glad.git GIT_TAG v2.0.8 SOURCE_SUBDIR cmake)
FetchContent_MakeAvailable(glfw glm glad)
glad_add_library(glad_gl_core_33 REPRODUCIBLE API gl:core=3.3)

add_library(MinecraftLogic
  src/core/WorldRepository.cpp src/core/Camera.cpp src/core/DayNightCycle.cpp src/core/SaveLoad.cpp src/core/SettingsState.cpp src/core/GameState.cpp
  src/player/Player.cpp src/player/PlayerPhysics.cpp src/player/PlayerCamera.cpp src/player/PlayerAnimator.cpp src/player/Inventory.cpp src/player/CraftingRecipe.cpp src/player/SurvivalState.cpp
  src/world/Chunk.cpp src/world/World.cpp src/world/WorldGenerator.cpp src/world/BlockProperties.cpp src/world/Pathfinder.cpp src/world/PassiveMobSystem.cpp
  src/utils/Raycaster.cpp)
target_include_directories(MinecraftLogic PUBLIC src PRIVATE ${glfw_SOURCE_DIR}/include)
target_link_libraries(MinecraftLogic PUBLIC glm::glm)

add_library(MinecraftRender
  src/core/Shader.cpp
  src/renderer/Mesh.cpp src/renderer/GpuMesh.cpp src/renderer/ChunkRenderer.cpp src/renderer/Renderer.cpp src/renderer/Texture.cpp src/renderer/CuboidMesh.cpp src/renderer/PlayerRenderer.cpp src/renderer/MobRenderer.cpp src/renderer/ParticleSystem.cpp src/renderer/DropSystem.cpp)
target_include_directories(MinecraftRender PUBLIC src)
target_link_libraries(MinecraftRender PUBLIC MinecraftLogic glad_gl_core_33 glfw)

add_library(MinecraftLib src/core/Application.cpp src/core/ApplicationMenus.cpp)
target_include_directories(MinecraftLib PUBLIC src)
target_link_libraries(MinecraftLib PUBLIC MinecraftRender)

add_executable(Minecraft src/main.cpp)
target_link_libraries(Minecraft PRIVATE MinecraftLib)

if(MSVC)
  foreach(target MinecraftLogic MinecraftRender MinecraftLib Minecraft)
    target_compile_options(${target} PRIVATE /W4 /permissive-)
  endforeach()
else()
  foreach(target MinecraftLogic MinecraftRender MinecraftLib Minecraft)
    target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
  endforeach()
endif()

include(CTest)
function(mc_add_test TARGET TEST_NAME LINK_TARGET)
  add_executable(${TARGET} tests/${TARGET}.cpp)
  target_link_libraries(${TARGET} PRIVATE ${LINK_TARGET})
  add_test(NAME ${TEST_NAME} COMMAND ${TARGET})
endfunction()

if(BUILD_TESTING)
  mc_add_test(PlayerPhysicsTests player_physics MinecraftLogic)
  mc_add_test(PlayerAnimatorTests player_animator MinecraftLogic)
  mc_add_test(InventoryTests inventory MinecraftLogic)
  mc_add_test(InventoryLayoutTests inventory_layout MinecraftLogic)
  mc_add_test(DayNightCycleTests day_night_cycle MinecraftLogic)
  mc_add_test(ChunkLightingTests chunk_lighting MinecraftLogic)
  mc_add_test(ChunkMeshGoldenTests chunk_mesh_golden MinecraftLogic)
  mc_add_test(WorldLightingTests world_lighting MinecraftLogic)
  mc_add_test(BlockPropertiesTests block_properties MinecraftLogic)
  mc_add_test(BlockRegistryTests block_registry MinecraftLogic)
  mc_add_test(MiningProgressTests mining_progress MinecraftLogic)
  mc_add_test(BreakModeTests break_mode MinecraftLogic)
  mc_add_test(WorldPersistenceTests world_persistence MinecraftLogic)
  mc_add_test(WorldStreamingTests world_streaming MinecraftLogic)
  mc_add_test(WorldGeneratorTests world_generator MinecraftLogic)
  mc_add_test(SaveLoadTests save_load MinecraftLogic)
  mc_add_test(GameStateTests game_state MinecraftLogic)
  mc_add_test(CraftingTests crafting MinecraftLogic)
  mc_add_test(ToolTierTests tool_tiers MinecraftLogic)
  mc_add_test(PlayerCameraTests player_camera MinecraftLogic)
  mc_add_test(SurvivalTests survival MinecraftLogic)
  mc_add_test(SprintStateTests sprint_state MinecraftLogic)
  mc_add_test(EatingStateTests eating_state MinecraftLogic)
  mc_add_test(LootTableTests loot_table MinecraftLogic)
  mc_add_test(PathfinderTests pathfinder MinecraftLogic)
  mc_add_test(PassiveMobTests passive_mobs MinecraftLogic)
  mc_add_test(MobMovementTests mob_movement MinecraftLogic)
  mc_add_test(FurnaceTickTests furnace_tick MinecraftLogic)
  mc_add_test(FurnacePersistenceTests furnace_persistence MinecraftLogic)
  mc_add_test(FurnaceRenderTests furnace_render MinecraftRender)
  set_tests_properties(furnace_render PROPERTIES SKIP_RETURN_CODE 77)
  mc_add_test(CookingApplicationTests cooking_application MinecraftLib)
  set_tests_properties(cooking_application PROPERTIES SKIP_RETURN_CODE 77)
  mc_add_test(MenuCoreTests menu_core MinecraftLogic)
  mc_add_test(SeedSneakTests seed_sneak MinecraftLogic)
  mc_add_test(MenusApplicationTests menus_application MinecraftLib)
  set_tests_properties(menus_application PROPERTIES SKIP_RETURN_CODE 77)
  mc_add_test(ChunkTests chunk MinecraftLogic)
  mc_add_test(FrustumTests frustum MinecraftLogic)
  mc_add_test(LightingEquivalenceTests lighting_equivalence MinecraftLogic)
  # Visual equivalence for culling + maxY (ported to the ChunkRenderer
  # architecture). GL-gated like the other GL tests; GLFW_INCLUDE_NONE lets
  # it build where system GL headers are missing.
  add_executable(VisualEquivalenceTests tests/VisualEquivalenceTests.cpp)
  target_include_directories(VisualEquivalenceTests PRIVATE src)
  target_compile_definitions(VisualEquivalenceTests PRIVATE GLFW_INCLUDE_NONE)
  target_link_libraries(VisualEquivalenceTests PRIVATE MinecraftRender glfw glad_gl_core_33)
  add_test(NAME visual_equivalence COMMAND VisualEquivalenceTests)
  set_tests_properties(visual_equivalence PROPERTIES SKIP_RETURN_CODE 77)
endif()

# Headless renderer benchmark (Sodium-style perf pass, ported to the
# ChunkRenderer architecture). Needs a GL context; run under Xvfb.
# Not part of ctest.
add_executable(PerfBench bench/perf_bench.cpp)
target_include_directories(PerfBench PRIVATE src)
target_compile_definitions(PerfBench PRIVATE GLFW_INCLUDE_NONE)
target_link_libraries(PerfBench PRIVATE MinecraftRender glfw glad_gl_core_33)
