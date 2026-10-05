// Visual equivalence for the perf pass (frustum culling + maxY-bounded meshing)
// on the ChunkRenderer architecture. GL-gated: returns 77 when no GL context
// is available (runs under Xvfb).
//
// Core invariant: culling and the maxY bound must be invisible. For every
// camera, the frame with culling ON is byte-compared against culling OFF,
// and the frame with the maxY bound ON against the bound forced OFF.
// Any pixel difference is a bug. llvmpipe verifies correctness only.
//
// Determinism: fixed seed (7), fixed camera list, no wall-clock or
// day/night dependence (the test shader uses only baked vertex shade).
// Failure artifacts (A/B/diff PPMs) go to visual-artifacts/ (gitignored),
// passed as argv[1] or defaulting to ./visual-artifacts.
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include "world/Block.h"
#include "world/Chunk.h"
#include "world/World.h"
#include "world/WorldGenerator.h"
#include "renderer/ChunkRenderer.h"
#include "renderer/Frustum.h"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
constexpr int W = 1280, H = 720;
constexpr std::uint32_t SEED = 7;
constexpr int VD = 8;
int g_failures = 0;
int g_comparisons = 0;
float g_maxTerrainFrac = 0.f;  // empty-scene guard: max non-sky fraction seen
std::filesystem::path g_artifacts;

#define CHECK(x) \
  do { \
    if (!(x)) throw std::runtime_error(std::string("check failed at line ") + std::to_string(__LINE__) + ": " #x); \
  } while (false)

GLuint compileShader(GLenum type, const char* src) {
  GLuint s = glCreateShader(type);
  glShaderSource(s, 1, &src, nullptr);
  glCompileShader(s);
  GLint ok = 0;
  glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
  if (!ok) {
    char log[1024];
    glGetShaderInfoLog(s, sizeof(log), nullptr, log);
    throw std::runtime_error(std::string("shader compile: ") + log);
  }
  return s;
}

struct GL {
  GLFWwindow* window = nullptr;
  GLuint fbo = 0, color = 0, depth = 0, prog = 0;
  GLint uVP = -1;
  GL() {
    if (!glfwInit()) throw std::runtime_error("glfwInit failed");
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    window = glfwCreateWindow(W, H, "visual-equivalence", nullptr, nullptr);
    if (!window) {
      glfwTerminate();
      throw std::runtime_error("glfwCreateWindow failed");
    }
    glfwMakeContextCurrent(window);
    if (!gladLoadGL(glfwGetProcAddress)) throw std::runtime_error("gladLoadGL failed");
    const char* vs =
        "#version 330 core\n"
        "layout(location=0) in vec3 aPos;layout(location=1) in vec2 aUV;layout(location=2) in float aShade;"
        "uniform mat4 uVP;out float vShade;"
        "void main(){vShade=aShade;gl_Position=uVP*vec4(aPos,1.0);}";
    const char* fs =
        "#version 330 core\n"
        "in float vShade;out vec4 o;"
        "void main(){o=vec4(vec3(0.32,0.52,0.28)*vShade,1.0);}";
    GLuint v = compileShader(GL_VERTEX_SHADER, vs), f = compileShader(GL_FRAGMENT_SHADER, fs);
    prog = glCreateProgram();
    glAttachShader(prog, v);
    glAttachShader(prog, f);
    glLinkProgram(prog);
    GLint ok = 0;
    glGetProgramiv(prog, GL_LINK_STATUS, &ok);
    CHECK(ok);
    glDeleteShader(v);
    glDeleteShader(f);
    uVP = glGetUniformLocation(prog, "uVP");
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glGenRenderbuffers(1, &color);
    glBindRenderbuffer(GL_RENDERBUFFER, color);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGB8, W, H);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, color);
    glGenRenderbuffers(1, &depth);
    glBindRenderbuffer(GL_RENDERBUFFER, depth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, W, H);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth);
    CHECK(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }
  ~GL() {
    if (prog) glDeleteProgram(prog);
    if (color) glDeleteRenderbuffers(1, &color);
    if (depth) glDeleteRenderbuffers(1, &depth);
    if (fbo) glDeleteFramebuffers(1, &fbo);
    if (window) glfwDestroyWindow(window);
    glfwTerminate();
  }
};

// The ChunkRenderer caches GPU meshes per world; it must outlive the A/B
// frames of one mesh state, and a fresh renderer is needed per World (chunk
// keys would collide across worlds).
void renderFrame(GL& gl, ChunkRenderer& renderer, World& world, const Frustum& frustum,
                 const glm::mat4& vp, std::vector<unsigned char>& out) {
  glBindFramebuffer(GL_FRAMEBUFFER, gl.fbo);
  glViewport(0, 0, W, H);
  glClearColor(0.45f, 0.65f, 1.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glUseProgram(gl.prog);
  glUniformMatrix4fv(gl.uVP, 1, GL_FALSE, &vp[0][0]);
  renderer.render(world, frustum);
  glFinish();
  CHECK(glGetError() == GL_NO_ERROR);
  glReadBuffer(GL_COLOR_ATTACHMENT0);
  glPixelStorei(GL_PACK_ALIGNMENT, 1);
  out.assign(W * H * 3, 0);
  glReadPixels(0, 0, W, H, GL_RGB, GL_UNSIGNED_BYTE, out.data());
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

Frustum everythingVisible() {
  Frustum f;
  for (auto& p : f.planes) p = glm::vec4(0.f, 0.f, 0.f, 1e9f);
  return f;
}

void writePPM(const std::filesystem::path& path, const std::vector<unsigned char>& px) {
  std::ofstream out(path, std::ios::binary);
  out << "P6\n" << W << ' ' << H << "\n255\n";
  for (int y = H - 1; y >= 0; --y) out.write(reinterpret_cast<const char*>(px.data() + y * W * 3), W * 3);
}

long compareFrames(const std::string& label, const std::vector<unsigned char>& a,
                   const std::vector<unsigned char>& b) {  ++g_comparisons;
  CHECK(a.size() == b.size());
  long diff = 0;
  for (size_t i = 0; i < a.size(); ++i)
    if (a[i] != b[i]) ++diff;
  if (diff == 0) {
    std::cout << "  PASS " << label << "\n";
  } else {
    ++g_failures;
    std::cout << "  FAIL " << label << " (" << diff << " bytes differ)\n";
    if (!g_artifacts.empty()) {
      std::filesystem::create_directories(g_artifacts);
      std::string safe = label;
      for (char& c : safe)
        if (c == ' ' || c == '/') c = '_';
      writePPM(g_artifacts / (safe + "_a.ppm"), a);
      writePPM(g_artifacts / (safe + "_b.ppm"), b);
      std::vector<unsigned char> d = a;
      for (size_t i = 0; i < d.size(); i += 3)
        if (a[i] != b[i] || a[i + 1] != b[i + 1] || a[i + 2] != b[i + 2]) {
          d[i] = 255;
          d[i + 1] = 0;
          d[i + 2] = 0;
        }
      writePPM(g_artifacts / (safe + "_diff.ppm"), d);
    }
  }
  return diff;
}

void drain(World& world, const glm::vec3& center) {
  world.setTaskBudgets(64, 64, 64);
  // do-while: a fresh world has no pending tasks until the first update()
  // queues generation.
  for (int i = 0; i < 20000; ++i) {
    world.update(center);
    if (world.pendingTaskCount() == 0) break;
  }
  CHECK(world.pendingTaskCount() == 0);
}

int chunkCoord(float v) {
  int c = static_cast<int>(std::floor(v)) / 16;
  if (static_cast<int>(std::floor(v)) < 0 && static_cast<int>(std::floor(v)) % 16 != 0) --c;
  return std::clamp(c, 0, 62);
}

bool allReady(const World& world, const glm::vec3& center, int vd) {
  int pcx = chunkCoord(center.x), pcz = chunkCoord(center.z);
  for (int cz = std::max(0, pcz - vd); cz <= std::min(62, pcz + vd); ++cz)
    for (int cx = std::max(0, pcx - vd); cx <= std::min(62, pcx + vd); ++cx)
      if (!world.isChunkReady(cx, cz)) return false;
  return true;
}

int surfaceY(World& world, int x, int z) {
  for (int y = 255; y >= 0; --y)
    if (world.getBlock(x, y, z) != BlockType::AIR) return y;
  return -1;
}

glm::mat4 cameraVP(const glm::vec3& eye, float yawDeg, float pitchDeg, float fovDeg) {
  float yaw = glm::radians(yawDeg), pitch = glm::radians(pitchDeg);
  glm::vec3 dir(std::sin(yaw) * std::cos(pitch), std::sin(pitch), -std::cos(yaw) * std::cos(pitch));
  glm::mat4 view = glm::lookAt(eye, eye + dir, glm::vec3(0, 1, 0));
  glm::mat4 proj = glm::perspective(glm::radians(fovDeg), float(W) / float(H), 0.1f, 1000.f);
  return proj * view;
}

struct Camera {
  glm::vec3 eye;
  float yaw, pitch, fov;
  std::string name;
  // Straight-up cameras legitimately see only sky; everything else must see
  // a non-trivial frame (per-pair floor, enforced in checkCamera).
  bool expectTerrain = true;
};

// Fraction of pixels that are not the sky-blue clear color (115,166,255).
// Guards against the classic lie: a drain-loop regression that leaves the
// world empty makes every A/B pair trivially identical (two blank frames).
long nonSkyPixels(const std::vector<unsigned char>& px) {
  long n = 0;
  for (size_t i = 0; i < px.size(); i += 3)
    if (!(px[i] == 115 && px[i + 1] == 166 && px[i + 2] == 255)) ++n;
  return n;
}

float terrainFraction(const std::vector<unsigned char>& px) {
  return float(nonSkyPixels(px)) / float(px.size() / 3);
}

// Per-pair floor: a terrain-expected camera whose frame is (nearly) all sky
// means the world failed to load — fail loudly here, not just at the suite
// level, so the failure points at the camera. 500 px ~= 0.05% of 1280x720.
constexpr long MIN_NONSKY_PIXELS = 500;

// Render A=cull,B=no-cull under both mesh states; compare cull<->nocull and
// bound-on<->bound-off. Caller guarantees bound-ON meshes on entry.
void checkCamera(GL& gl, ChunkRenderer& renderer, World& world, const Camera& cam) {
  glm::mat4 vp = cameraVP(cam.eye, cam.yaw, cam.pitch, cam.fov);
  Frustum cull = Frustum::fromMatrix(vp), open = everythingVisible();
  std::vector<unsigned char> a, b, c, d;
  renderFrame(gl, renderer, world, cull, vp, a);
  renderFrame(gl, renderer, world, open, vp, b);
  compareFrames(cam.name + " | cull on vs off (bound on)", a, b);
  g_maxTerrainFrac = std::max(g_maxTerrainFrac, terrainFraction(a));
  world.debugRebuildMeshes(false);
  renderFrame(gl, renderer, world, cull, vp, c);
  renderFrame(gl, renderer, world, open, vp, d);
  compareFrames(cam.name + " | cull on vs off (bound off)", c, d);
  compareFrames(cam.name + " | maxY bound on vs off", a, c);
  // Per-pair non-trivial-pixel floor: two identical blank frames must never
  // pass. Straight-up (pitch +89) cameras are exempt — pure sky is expected.
  if (cam.expectTerrain) {
    long n = nonSkyPixels(a);
    if (n < MIN_NONSKY_PIXELS) {
      ++g_failures;
      std::cout << "  FAIL " << cam.name << " | trivial frame: only " << n << " non-sky pixels\n";
    }
  }
  world.debugRebuildMeshes(true);
}
}  // namespace

namespace {
// ---- Task 2: camera sweep ----
std::vector<Camera> buildSweep(World& world) {
  std::vector<Camera> cams;
  glm::vec3 base(500.5f, 0, 500.5f);
  base.y = float(surfaceY(world, 500, 500)) + 2.f;
  for (int yaw = 0; yaw < 360; yaw += 15)
    cams.push_back({base, float(yaw), 0.f, 70.f, "yaw-" + std::to_string(yaw)});
  for (float pitch : {-89.f, -45.f, 0.f, 45.f, 89.f})
    for (float yaw : {0.f, 90.f, 180.f})
      cams.push_back({base, yaw, pitch, 70.f,
                      "pitch-" + std::to_string(int(pitch)) + "-yaw-" + std::to_string(int(yaw)),
                      /*expectTerrain=*/pitch != 89.f});
  for (float fov : {30.f, 70.f, 110.f, 140.f})
    cams.push_back({base, 45.f, -10.f, fov, "fov-" + std::to_string(int(fov))});
  // hilltop / valley: scan the neighbourhood for extremes
  int hiX = 500, hiZ = 500, loX = 500, loZ = 500, hi = -1, lo = 256;
  for (int z = 460; z <= 540; z += 4)
    for (int x = 460; x <= 540; x += 4) {
      int s = surfaceY(world, x, z);
      if (s > hi) {
        hi = s;
        hiX = x;
        hiZ = z;
      }
      if (s < lo) {
        lo = s;
        loX = x;
        loZ = z;
      }
    }
  cams.push_back({{hiX + .5f, hi + 3.f, hiZ + .5f}, 45.f, -15.f, 70.f, "hilltop"});
  cams.push_back({{loX + .5f, lo + 3.f, loZ + .5f}, 45.f, -15.f, 70.f, "valley"});
  // chunk corner (x%16==0, z%16==0), chunk edge, boundary-look along x=512
  cams.push_back({{512.f, surfaceY(world, 512, 512) + 2.f, 512.f}, 45.f, -15.f, 70.f, "chunk-corner"});
  cams.push_back({{512.f, surfaceY(world, 512, 520) + 2.f, 520.5f}, 45.f, -15.f, 70.f, "chunk-edge"});
  cams.push_back({{512.f, surfaceY(world, 512, 500) + 3.f, 500.5f}, 90.f, -10.f, 70.f, "boundary-look"});
  // underground: find a cave pocket (3+ vertical air below the surface)
  bool found = false;
  for (int cz = 28; cz <= 34 && !found; ++cz)
    for (int cx = 28; cx <= 34 && !found; ++cx)
      for (int z = cz * 16; z < cz * 16 + 16 && !found; ++z)
        for (int x = cx * 16; x < cx * 16 + 16 && !found; ++x) {
          int s = surfaceY(world, x, z);
          for (int y = s - 3; y >= 5; --y)
            if (world.getBlock(x, y, z) == BlockType::AIR && world.getBlock(x, y - 1, z) == BlockType::AIR &&
                world.getBlock(x, y + 1, z) == BlockType::AIR) {
              cams.push_back({{x + .5f, y + 1.f, z + .5f}, 45.f, 0.f, 70.f, "cave"});
              found = true;
              break;
            }
        }
  if (!found) std::cout << "  NOTE: no cave pocket found near center; skipping cave camera\n";
  return cams;
}

void task2_sweep(GL& gl, ChunkRenderer& renderer, World& world) {
  std::cout << "Task 2: camera sweep (" << VD << " view distance)\n";
  auto cams = buildSweep(world);
  std::cout << "  cameras: " << cams.size() << "\n";
  for (const auto& cam : cams) checkCamera(gl, renderer, world, cam);
  // Empty-scene guard (his bar, pre-push req 1): at least one camera must see
  // mostly terrain. A drain regression that loads no chunks would otherwise
  // pass every comparison (two blank frames are always identical). Observed
  // ~0.70 non-sky on seed 7; the floor is 0.7.
  std::cout << "  max terrain fraction seen: " << g_maxTerrainFrac << "\n";
  CHECK(g_maxTerrainFrac > 0.7f);
}

// ---- Task 3: maxY edit scenarios ----
struct Plateau {
  int cx, cz, topY, count;
};

int bruteMaxY(World& world, int cx, int cz) {
  for (int y = 255; y >= 0; --y)
    for (int z = 0; z < 16; ++z)
      for (int x = 0; x < 16; ++x)
        if (world.getBlock(cx * 16 + x, y, cz * 16 + z) != BlockType::AIR) return y;
  return -1;
}

int reportedMaxY(World& world, int cx, int cz) {
  for (const auto& info : world.debugChunkInfo())
    if (info.cx == cx && info.cz == cz) return info.maxY;
  return -2;
}

void task3_edits(GL& gl, ChunkRenderer& renderer, World& world, const glm::vec3& center) {
  std::cout << "Task 3: maxY edit scenarios\n";
  // Known seed-7 scenarios (worldgen-dependent). If the generator changes,
  // these CHECKs fail loudly instead of silently testing some other chunk.
  // Plateau: chunk (23,39) has exactly 3 blocks at y=59.
  // Tree: a LEAVES block sits at the top of (383,59,638), chunk (23,39).
  {
    int n = 0;
    for (int z = 0; z < 16; ++z)
      for (int x = 0; x < 16; ++x)
        if (world.getBlock(23 * 16 + x, 59, 39 * 16 + z) != BlockType::AIR) ++n;
    CHECK(n == 3);
    int top = bruteMaxY(world, 23, 39);
    CHECK(top == 59);
  }
  Plateau pl{23, 39, 59, 3};
  std::cout << "  plateau: chunk (" << pl.cx << "," << pl.cz << ") top y=" << pl.topY << " blocks=" << pl.count
            << "\n";
  // locate one top block to mine
  int ex = -1, ez = -1;
  for (int z = 0; z < 16 && ex < 0; ++z)
    for (int x = 0; x < 16 && ex < 0; ++x)
      if (world.getBlock(pl.cx * 16 + x, pl.topY, pl.cz * 16 + z) != BlockType::AIR) {
        ex = pl.cx * 16 + x;
        ez = pl.cz * 16 + z;
      }
  glm::vec3 look(pl.cx * 16 + 8.f, pl.topY, pl.cz * 16 + 8.f);
  glm::vec3 eye = look + glm::vec3(26.f, 14.f, 26.f);
  glm::mat4 vp = glm::perspective(glm::radians(60.f), float(W) / float(H), 0.1f, 1000.f) *
                 glm::lookAt(eye, look, glm::vec3(0, 1, 0));
  Frustum cull = Frustum::fromMatrix(vp);
  std::vector<unsigned char> before, after, refFrame;
  renderFrame(gl, renderer, world, cull, vp, before);

  CHECK(world.setBlock(ex, pl.topY, ez, BlockType::AIR));
  drain(world, center);
  // maxY must match brute force after the edit
  int trueTop = bruteMaxY(world, pl.cx, pl.cz);
  int reported = reportedMaxY(world, pl.cx, pl.cz);
  std::cout << "  after mining one top block: maxY=" << reported << " brute=" << trueTop << "\n";
  CHECK(reported == trueTop);
  renderFrame(gl, renderer, world, cull, vp, after);

  // fresh world, same edit, maxY bound disabled: must render identically
  World refWorld(SEED);
  refWorld.setViewDistance(VD);
  drain(refWorld, center);
  CHECK(refWorld.setBlock(ex, pl.topY, ez, BlockType::AIR));
  drain(refWorld, center);
  refWorld.debugRebuildMeshes(false);
  ChunkRenderer refRenderer;
  renderFrame(gl, refRenderer, refWorld, cull, vp, refFrame);
  compareFrames("edit-plateau | edited bound-on vs fresh bound-off", after, refFrame);

  // variant: single-block peak mined (scan for a chunk whose top level has 1 block)
  int px = -1, pz = -1;
  Plateau peak{-1, -1, -1, 0};
  for (const auto& info : world.debugChunkInfo()) {
    int n = 0;
    for (int z = 0; z < 16; ++z)
      for (int x = 0; x < 16; ++x)
        if (world.getBlock(info.cx * 16 + x, info.maxY, info.cz * 16 + z) != BlockType::AIR) ++n;
    if (n == 1) {
      for (int z = 0; z < 16 && px < 0; ++z)
        for (int x = 0; x < 16 && px < 0; ++x)
          if (world.getBlock(info.cx * 16 + x, info.maxY, info.cz * 16 + z) != BlockType::AIR) {
            px = info.cx * 16 + x;
            pz = info.cz * 16 + z;
            peak = {info.cx, info.cz, info.maxY, 1};
          }
      break;
    }
  }
  if (px >= 0) {
    glm::vec3 pl2(peak.cx * 16 + 8.f, peak.topY, peak.cz * 16 + 8.f);
    glm::vec3 eye2 = pl2 + glm::vec3(26.f, 14.f, 26.f);
    glm::mat4 vp2 = glm::perspective(glm::radians(60.f), float(W) / float(H), 0.1f, 1000.f) *
                    glm::lookAt(eye2, pl2, glm::vec3(0, 1, 0));
    Frustum cull2 = Frustum::fromMatrix(vp2);
    CHECK(world.setBlock(px, peak.topY, pz, BlockType::AIR));
    drain(world, center);
    int t2 = bruteMaxY(world, peak.cx, peak.cz), r2 = reportedMaxY(world, peak.cx, peak.cz);
    std::cout << "  single peak mined: maxY=" << r2 << " brute=" << t2 << "\n";
    CHECK(r2 == t2);
    std::vector<unsigned char> f1, f2;
    renderFrame(gl, renderer, world, cull2, vp2, f1);
    world.debugRebuildMeshes(false);
    renderFrame(gl, renderer, world, cull2, vp2, f2);
    compareFrames("edit-peak | maxY bound on vs off", f1, f2);
    world.debugRebuildMeshes(true);
  } else {
    std::cout << "  NOTE: no single-block peak found; skipping\n";
  }

  // variant: place a block above the current top
  {
    int sx = 500, sz = 500, colTop = -1;
    for (int y = 255; y >= 0; --y)
      if (world.getBlock(sx, y, sz) != BlockType::AIR) {
        colTop = y;
        break;
      }
    CHECK(colTop >= 0 && colTop + 2 < 256);
    glm::vec3 pl3(sx + .5f, colTop, sz + .5f);
    glm::vec3 eye3 = pl3 + glm::vec3(20.f, 12.f, 20.f);
    glm::mat4 vp3 = glm::perspective(glm::radians(60.f), float(W) / float(H), 0.1f, 1000.f) *
                    glm::lookAt(eye3, pl3, glm::vec3(0, 1, 0));
    Frustum cull3 = Frustum::fromMatrix(vp3);
    CHECK(world.setBlock(sx, colTop + 1, sz, BlockType::STONE));
    CHECK(world.setBlock(sx, colTop + 2, sz, BlockType::STONE));
    drain(world, center);
    int t3 = bruteMaxY(world, chunkCoord(float(sx)), chunkCoord(float(sz)));
    int r3 = reportedMaxY(world, chunkCoord(float(sx)), chunkCoord(float(sz)));
    std::cout << "  placed above top: maxY=" << r3 << " brute=" << t3 << "\n";
    int colTopAfter = -1;
    for (int y = 255; y >= 0; --y)
      if (world.getBlock(sx, y, sz) != BlockType::AIR) {
        colTopAfter = y;
        break;
      }
    CHECK(r3 == t3 && colTopAfter == colTop + 2 && r3 >= colTop + 2);
    std::vector<unsigned char> f1, f2;
    renderFrame(gl, renderer, world, cull3, vp3, f1);
    world.debugRebuildMeshes(false);
    renderFrame(gl, renderer, world, cull3, vp3, f2);
    compareFrames("edit-place-above | maxY bound on vs off", f1, f2);
    world.debugRebuildMeshes(true);
  }

  // variant: break a tree's top leaf (anchored: (383,59,638) is LEAVES on seed 7)
  {
    CHECK(world.getBlock(383, 59, 638) == BlockType::LEAVES);
    const int wx = 383, wz = 638, wy = 59;
    glm::vec3 pl4(wx + .5f, wy, wz + .5f);
    glm::vec3 eye4 = pl4 + glm::vec3(24.f, 13.f, 24.f);
    glm::mat4 vp4 = glm::perspective(glm::radians(60.f), float(W) / float(H), 0.1f, 1000.f) *
                    glm::lookAt(eye4, pl4, glm::vec3(0, 1, 0));
    Frustum cull4 = Frustum::fromMatrix(vp4);
    CHECK(world.setBlock(wx, wy, wz, BlockType::AIR));
    drain(world, center);
    int t4 = bruteMaxY(world, 23, 39), r4 = reportedMaxY(world, 23, 39);
    std::cout << "  tree top leaf broken at (" << wx << "," << wy << "," << wz << "): maxY=" << r4
              << " brute=" << t4 << "\n";
    CHECK(r4 == t4);
    std::vector<unsigned char> f1, f2;
    renderFrame(gl, renderer, world, cull4, vp4, f1);
    world.debugRebuildMeshes(false);
    renderFrame(gl, renderer, world, cull4, vp4, f2);
    compareFrames("edit-tree-leaf | maxY bound on vs off", f1, f2);
    world.debugRebuildMeshes(true);
  }
}

// ---- Task 4: border and streaming (deferred lighting) ----
void checkAllReady(GL& gl, ChunkRenderer& renderer, const World& world, const glm::vec3& center,
                   const std::string& where) {
  int pcx = chunkCoord(center.x), pcz = chunkCoord(center.z);
  int missing = 0;
  for (int cz = std::max(0, pcz - VD); cz <= std::min(62, pcz + VD); ++cz)
    for (int cx = std::max(0, pcx - VD); cx <= std::min(62, pcx + VD); ++cx)
      if (!world.isChunkReady(cx, cz)) ++missing;
  int dirty = 0;
  for (const auto& info : world.debugChunkInfo())
    if (info.meshDirty || info.lightingDirty) ++dirty;
  std::cout << "  " << where << ": missing=" << missing << " dirty=" << dirty << "\n";
  CHECK(missing == 0 && dirty == 0);
}

void task4_streaming(GL& gl, ChunkRenderer& renderer, World& world, const glm::vec3& center) {
  std::cout << "Task 4: border and streaming\n";
  checkAllReady(gl, renderer, world, center, "initial drain");
  // teleport near each world edge (world spans chunks 0..62; a 40-chunk jump
  // would leave the world, so the edges are the meaningful teleport targets)
  for (const glm::vec3& tp : {glm::vec3(60.5f, 70.f, 500.5f), glm::vec3(948.5f, 70.f, 500.5f)}) {
    drain(world, tp);
    int pcx = chunkCoord(tp.x);
    checkAllReady(gl, renderer, world, tp, pcx < 31 ? "teleport west edge" : "teleport east edge");
    glm::vec3 eye(tp.x, surfaceY(world, int(tp.x), int(tp.z)) + 2.f, tp.z);
    Camera cam{eye, 45.f, -10.f, 70.f, "teleport"};
    glm::mat4 vp = cameraVP(cam.eye, cam.yaw, cam.pitch, cam.fov);
    std::vector<unsigned char> a, b;
    renderFrame(gl, renderer, world, Frustum::fromMatrix(vp), vp, a);
    renderFrame(gl, renderer, world, everythingVisible(), vp, b);
    compareFrames(cam.name + " | cull on vs off after teleport", a, b);
  }
  // walk one chunk at a time; measure frames until all chunks ready
  drain(world, center);
  int observedMax = 0;
  const glm::vec3 dirs[4] = {{16, 0, 0}, {-16, 0, 0}, {0, 0, 16}, {0, 0, -16}};
  glm::vec3 pos = center;
  for (int d = 0; d < 4; ++d)
    for (int step = 0; step < 3; ++step) {
      pos += dirs[d];
      int frames = 0;
      do {
        world.update(pos);
        ++frames;
      } while (!allReady(world, pos, VD) && frames < 500);
      observedMax = std::max(observedMax, frames);
      CHECK(frames < 500);
    }
  std::cout << "  walk: observed max frames to all-ready = " << observedMax << "\n";
  CHECK(observedMax <= 60);
  drain(world, center);
  checkAllReady(gl, renderer, world, center, "back at start");
}
}  // namespace

int main(int argc, char** argv) {
  if (argc > 1) g_artifacts = argv[1];
  // GL gate: same convention as the other GL tests (ctest SKIP_RETURN_CODE 77)
  if (!glfwInit()) return 77;
  glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
  GLFWwindow* probe = glfwCreateWindow(64, 64, "probe", nullptr, nullptr);
  if (!probe) {
    glfwTerminate();
    return 77;
  }
  glfwDestroyWindow(probe);
  glfwTerminate();
  try {
    GL gl;
    ChunkRenderer renderer;
    World world(SEED);
    world.setViewDistance(VD);
    glm::vec3 center(500.5f, 70.f, 500.5f);
    drain(world, center);
    // 17x17 chunks at vd=8, all in-bounds. Guards the drain: an empty world
    // would make every A/B pair trivially identical.
    CHECK(world.loadedChunkCount() == 289);
    std::cout << "loaded chunks: " << world.loadedChunkCount() << "\n";
    task2_sweep(gl, renderer, world);
    task3_edits(gl, renderer, world, center);
    task4_streaming(gl, renderer, world, center);
  } catch (const std::exception& e) {
    std::cerr << "ERROR: " << e.what() << "\n";
    return 1;
  }
  std::cout << "\ncomparisons: " << g_comparisons << ", failures: " << g_failures << "\n";
  return g_failures == 0 ? 0 : 1;
}
