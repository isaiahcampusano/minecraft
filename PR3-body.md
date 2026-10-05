# PR #3: Visual equivalence suite for culling + maxY (test/visual-equivalence → main)

## What it proves
Culling and the maxY bound are **invisible**: 153/153 A/B pixel comparisons byte-identical, 0 failures — 49-camera sweep (24 yaw, 15 pitch, 4 FOV incl. 140; hilltop/valley/chunk-corner/chunk-edge/boundary-look/cave) × (cull on/off, maxY bound on/off); maxY edit scenarios (plateau mine, single-peak mine, place-above-top, tree-leaf break — all maxY == brute force, edited frame == fresh-world bound-off frame); deferred-lighting border cases (teleport to chunk-0/chunk-62 edges, walk 1 chunk at a time — observed max 1 frame to all-ready, no stuck chunks).

## Scope of the claim (llvmpipe limit)
This suite runs headless under Xvfb/llvmpipe. It proves **correctness** (culling and the bound change no pixels), and says **nothing about driver behavior or frame pacing** — GPU-driver differences and real-display pacing still need the 2-min manual check on real hardware.

## Anti-lie guards (per pre-push review)
- Per-camera trivial-frame floor: any terrain-expected camera rendering <500 non-sky pixels fails loudly (two identical blank frames can never pass).
- Suite-level floor: max terrain fraction must exceed 0.7 (observed ~0.70–1.0 on seed 7).
- `CHECK(world.loadedChunkCount() == 289)` before comparisons — a drain-loop regression fails at the load check, not the comparisons. Negative-tested with `drain()` no-op'd: fails loudly instead of passing 153/153 trivially.

## Test hooks
- `World::debugRemesh(bool)` / `World::debugChunkInfo()` — `debug*`-named, test-only.
- `Chunk::generateMesh(..., bool useMaxYBound = true)` — defaulted; the default call is the production code path. Byte-identical bound-on vs bound-off frames confirm the same faces.

## Determinism
Fixed seed (7), fixed camera list, no wall-clock or day/night dependence (test shader uses baked vertex shade only). Failure artifacts (A/B/diff PPMs) go to `visual-artifacts/` (gitignored). Known seed-7 scenarios are commented in the suite: plateau chunk (23,39) has exactly 3 blocks at y=59; a LEAVES block sits at the top of (383,59,638).

## Gating
GL-gated like the other GL tests: returns 77 with no GL context, so plain `ctest` skips it headless; runs under Xvfb.
