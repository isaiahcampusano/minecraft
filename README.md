
<img width="1267" height="707" alt="image" src="https://github.com/user-attachments/assets/2ac4d189-b46b-4f08-a8bd-7f2fcebf6afa" />



---
```markdown
# Minecraft Superflat
A small Minecraft-style voxel demo in C++17 and OpenGL 3.3.

Streams 16×256×16 chunks over a finite 1000×1000 world with varied superflat terrain, full oak trees, first-person movement, collision, inventory management, persistent block editing, and **passive mobs**.

## Features

- **Terrain** — varied Bedrock + Stone + Dirt superflat stack with full oak trees that span chunk boundaries; broken leaves have a 0.5% chance to drop an apple.
- **Passive Mobs** — cows, pigs, and sheep spawn naturally in small packs and persist with the world. Adults idle and wander; babies follow nearby adults; damaged animals flee; sheep graze grass into dirt. Mobs have terrain-aware movement, attack hitboxes, mature after 20 minutes, and drop species-specific raw food and materials when killed. Drops remain in the world until collected.
- **Combat & Tools** — left-click attacks a mob within 4.5 blocks; otherwise, hold to mine. Harder materials take longer. Wooden and stone tools mine faster, lose durability, and stone/cobblestone only drop with a wood-or-better pickaxe.
- **Survival** — 20 health, 20 hunger, and 5 hidden saturation. Sprinting and jumping build exhaustion (consumes saturation first). Apples restore 4 hunger and 2.4 saturation. Natural healing at ≥18 hunger; starvation deals damage at 0 hunger. Falls over 3 blocks deal damage. Death clears inventory (world edits persist) and respawns you near world center.
- **Inventory & Crafting** — 27-slot backpack, 2×2 personal crafting grid, creative block palette, and crafting table support for 3×3 recipes. Logs → planks → sticks/crafting tables → wooden and stone tools.
- **Day-Night Cycle** — procedural 12‑minute cycle changes sky, fog, and terrain lighting; propagated skylight darkens holes, overhangs, and covered blocks.
- **Smooth Chunk Streaming** — terrain generation, lighting, and mesh uploads are deferred across frames and processed nearest-first. Unchanged lighting is reused, and existing meshes remain visible while edits rebuild.
- **Persistence** — world edits, inventory, survival state, and passive mobs save on exit. New saves use `MCv4`; existing `MCv2`/`MCv3` saves migrate automatically.
- **Texture Atlas** — generated in memory; no external asset files required.

## Build

Dependencies are downloaded by CMake during configuration.

```sh
cmake -S . -B build
cmake --build build --config Release
```

Run `Minecraft` from the build output directory.

## Controls

| Key | Action |
|-----|--------|
| `WASD` | Move |
| `Left Shift` / double-tap `W` | Toggle sprint (requires >6 hunger) |
| `Space` | Jump / fly upward |
| `Left Control` | Fly downward |
| `Mouse` | Look |
| `Scroll` | Change field of view |
| `Left Click` | Attack mob (within 4.5 blocks) or mine block |
| `Right Click` | Place selected block / eat apple (hold 1.6s) |
| `1–9` | Select hotbar slot |
| `E` | Open backpack, crafting grid, and creative palette |
| `Right Click` (crafting table) | Open 3×3 crafting grid |
| `G` | Toggle break mode: `SINGLE` / `FULL AUTO` |
| `F` | Toggle flying |
| `F5` | Switch first/third person |
| `Escape` | Release/capture mouse |

## Save Location

- Windows: `%APPDATA%\MinecraftClone\save.dat`
- Linux/macOS: `~/.minecraftclone/save.dat`

A missing, unsupported, or corrupted save is ignored and starts a fresh world.
```

---

**Key changes:**
- **Concise** — cut the wall-of-text description into clean sections.
- **Passive mobs featured** — moved to the top of the Features list with a dedicated paragraph.
- **Controls table** — much easier to scan than the original run-on list.
- **Save location** — kept at the bottom for quick reference.

You can copy this directly into your `README.md`. The build instructions, controls, and save path remain accurate to your repo.
