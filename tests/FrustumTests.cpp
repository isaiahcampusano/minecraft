// Tests for Frustum plane extraction and AABB intersection.
#include "renderer/Frustum.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <iostream>

namespace {
int fail(const char* message) {
  std::cerr << message << '\n';
  return 1;
}
bool near(float a, float b) {
  return std::fabs(a - b) < 1e-4f;
}
}  // namespace

int main() {
  // Orthographic: direct plane-value check plus inside/outside boxes.
  {
    const Frustum f = Frustum::fromMatrix(glm::ortho(-10.f, 10.f, -10.f, 10.f, 0.1f, 100.f));
    // Left plane of this ortho is x >= -10, i.e. normalized (1,0,0,10).
    if (!near(f.planes[0].x, 1.f) || !near(f.planes[0].w, 10.f)) return fail("ortho left plane wrong");
    if (!f.intersects({-1.f, -1.f, -50.f}, {1.f, 1.f, -40.f})) return fail("ortho: centred box culled");
    if (f.intersects({50.f, 50.f, -50.f}, {51.f, 51.f, -49.f})) return fail("ortho: distant box kept");
    if (f.intersects({-1.f, -1.f, 5.f}, {1.f, 1.f, 6.f})) return fail("ortho: box behind camera kept");
  }
  // Perspective 90 deg, camera at origin looking down -Z.
  {
    const Frustum f =
        Frustum::fromMatrix(glm::perspective(glm::radians(90.f), 1.f, 1.f, 10.f));
    if (!f.intersects({-0.5f, -0.5f, -5.5f}, {0.5f, 0.5f, -4.5f})) return fail("persp: centred box culled");
    if (f.intersects({-0.5f, -0.5f, 4.5f}, {0.5f, 0.5f, 5.5f})) return fail("persp: box behind camera kept");
    if (f.intersects({-0.5f, -0.5f, -15.5f}, {0.5f, 0.5f, -14.5f})) return fail("persp: box past far plane kept");
    if (f.intersects({-0.5f, -0.5f, -0.9f}, {0.5f, 0.5f, -0.1f})) return fail("persp: box before near plane kept");
    // Straddling the near plane: partially visible, must be kept (conservative).
    if (!f.intersects({-0.5f, -0.5f, -5.f}, {0.5f, 0.5f, -0.5f})) return fail("persp: straddling box culled");
  }
  // Wide FOV: boxes near the frustum edges must not be dropped.
  {
    const Frustum f =
        Frustum::fromMatrix(glm::perspective(glm::radians(120.f), 16.f / 9.f, 0.1f, 100.f));
    // At z=-10: half-width = tan(60)*10*16/9 ~= 30.8, half-height ~= 17.3.
    if (!f.intersects({24.f, -1.f, -11.f}, {26.f, 1.f, -9.f})) return fail("wide: edge box culled");
    if (f.intersects({34.f, -1.f, -11.f}, {36.f, 1.f, -9.f})) return fail("wide: outside box kept");
    if (!f.intersects({-1.f, 14.f, -11.f}, {1.f, 16.f, -9.f})) return fail("wide: top-edge box culled");
    if (f.intersects({-1.f, 25.f, -11.f}, {1.f, 27.f, -9.f})) return fail("wide: above-frustum box kept");
  }
  std::cout << "frustum tests passed\n";
  return 0;
}
