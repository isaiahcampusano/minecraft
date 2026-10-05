#pragma once
#include <glm/glm.hpp>

// Six-plane view frustum for chunk culling, extracted from the combined
// projection*view matrix. AABB intersection uses the positive-vertex test,
// which is conservative: an intersecting box is never reported as outside.
struct Frustum {
  // Plane i is (a,b,c,d); the inside half-space is ax+by+cz+d >= 0.
  glm::vec4 planes[6];

  static Frustum fromMatrix(const glm::mat4& clip) {
    Frustum f;
    // Rows of the column-major matrix.
    const glm::vec4 row0(clip[0][0], clip[1][0], clip[2][0], clip[3][0]);
    const glm::vec4 row1(clip[0][1], clip[1][1], clip[2][1], clip[3][1]);
    const glm::vec4 row2(clip[0][2], clip[1][2], clip[2][2], clip[3][2]);
    const glm::vec4 row3(clip[0][3], clip[1][3], clip[2][3], clip[3][3]);
    f.planes[0] = row3 + row0;  // left
    f.planes[1] = row3 - row0;  // right
    f.planes[2] = row3 + row1;  // bottom
    f.planes[3] = row3 - row1;  // top
    f.planes[4] = row3 + row2;  // near
    f.planes[5] = row3 - row2;  // far
    for (auto& p : f.planes) p /= glm::length(glm::vec3(p));
    return f;
  }

  bool intersects(const glm::vec3& mn, const glm::vec3& mx) const {
    for (const auto& p : planes) {
      const glm::vec3 pv(p.x >= 0 ? mx.x : mn.x, p.y >= 0 ? mx.y : mn.y, p.z >= 0 ? mx.z : mn.z);
      if (glm::dot(glm::vec3(p), pv) + p.w < 0) return false;
    }
    return true;
  }
};
