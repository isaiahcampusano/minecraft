// Headless renderer benchmark for the Sodium-style performance pass.
// Requires a GL context (Mesh::update uploads to the GPU), so run under Xvfb:
//   Xvfb :99 -screen 0 1280x720x24 &
//   DISPLAY=:99 ./build/PerfBench
#include "world/World.h"
#include "world/WorldGenerator.h"
#include "renderer/ChunkRenderer.h"
#include "renderer/Frustum.h"
#include "utils/Perf.h"
#include <GLFW/glfw3.h>
#include <glad/gl.h>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <vector>

namespace {
double nowMs() {
  return std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

// Run update() until the task queue drains. Returns updates used.
int drain(World& world, const glm::vec3& center, int cap = 6000) {
  int frames = 0;
  do {
    world.update(center);
    ++frames;
  } while (world.pendingTaskCount() > 0 && frames < cap);
  return frames;
}
}  // namespace

int main() {
  if (!glfwInit()) {
    std::fprintf(stderr, "glfwInit failed\n");
    return 1;
  }
  glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  GLFWwindow* win = glfwCreateWindow(1280, 720, "perf-bench", nullptr, nullptr);
  if (!win) {
    std::fprintf(stderr, "glfwCreateWindow failed\n");
    return 1;
  }
  glfwMakeContextCurrent(win);
  if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress)) {
    std::fprintf(stderr, "gladLoadGL failed\n");
    return 1;
  }

  const glm::vec3 center{500.5f, 70.0f, 500.5f};
  for (int vd : {4, 8}) {
    std::printf("\n===== view distance %d =====\n", vd);

    // Phase 1: per-chunk costs, drained with generous budgets.
    Perf::reset();
    World world(7);
    world.setViewDistance(vd);
    world.setTaskBudgets(32, 32, 32);
    const double t0 = nowMs();
    const int frames = drain(world, center);
    std::printf("drain: %d updates, %.0f ms wall, %zu chunks\n", frames, nowMs() - t0,
                world.loadedChunkCount());
    Perf::report();

    // Phase 2: edit burst - break 40 surface blocks in a row, measure relight+remesh.
    Perf::reset();
    for (int i = 0; i < 40; ++i) {
      const int x = 500 + i, z = 500;
      const int s = WorldGenerator::surfaceHeightAt(x, z, world.seed());
      world.setBlock(x, s, z, BlockType::AIR);
    }
    world.setTaskBudgets(32, 32, 32);
    const double e0 = nowMs();
    const int eframes = drain(world, center);
    std::printf("edit burst (40 breaks): %d updates, %.0f ms wall\n", eframes, nowMs() - e0);
    Perf::report();

    // Phase 3: paced stalls - fresh world, production 1/1/1 budgets, time every update.
    Perf::reset();
    World pacedWorld(7);
    pacedWorld.setViewDistance(vd);
    pacedWorld.setTaskBudgets(1, 1, 1);
    std::vector<double> samples;
    samples.reserve(900);
    for (int i = 0; i < 900; ++i) {
      const double u0 = nowMs();
      pacedWorld.update(center);
      samples.push_back(nowMs() - u0);
    }
    std::sort(samples.begin(), samples.end());
    double total = 0;
    for (double s : samples) total += s;
    std::printf("paced update() x900: avg=%.3fms p50=%.3fms p95=%.3fms max=%.3fms\n", total / samples.size(),
                samples[samples.size() / 2], samples[samples.size() * 95 / 100], samples.back());
    Perf::report();

    // Phase 4: frustum culling - eye-level camera, count drawn vs loaded chunks.
    // (No game shader is bound, so this measures draw-call submission only.)
    Perf::reset();
    {
      const glm::vec3 eye{500.5f, 75.f, 500.5f};
      const glm::mat4 view = glm::lookAt(eye, eye + glm::vec3(1.f, -0.08f, 0.f), glm::vec3(0.f, 1.f, 0.f));
      const glm::mat4 proj = glm::perspective(glm::radians(70.f), 16.f / 9.f, 0.1f, 1000.f);
      const Frustum frustum = Frustum::fromMatrix(proj * view);
      ChunkRenderer renderer;
      const double r0 = nowMs();
      for (int i = 0; i < 200; ++i) renderer.render(world, frustum);
      std::printf("render x200 with frustum: %.2f ms total (%.3f ms/frame), %zu chunks loaded\n",
                  nowMs() - r0, (nowMs() - r0) / 200, world.loadedChunkCount());
      Perf::report();
    }
  }
  glfwTerminate();
  return 0;
}
