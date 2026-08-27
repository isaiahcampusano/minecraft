# Minecraft Superflat

A small Minecraft-style voxel demo in C++17 and OpenGL 3.3. It streams 16×256×16 chunks over a finite 1000×1000 world, generates a varied Bedrock + Stone + Dirt superflat stack with full oak trees, meshes visible faces, and supports first-person movement, collision, inventory management, and persistent block editing.

## Build

Dependencies are downloaded by CMake during configuration.

```sh
cmake -S . -B build
cmake --build build --config Release
```

Run `Minecraft` from the build output directory.

## Controls

- **WASD** move, **Space/Left Shift** fly up/down
- **Mouse** look, **scroll** changes field of view
- **Hold left click** mines a block; harder materials take longer
- **Right click** places the selected hotbar block on the selected face
- **1–9** selects a hotbar slot
- **E** opens the backpack and creative block palette; click slots to swap stacks and scroll to change palette pages
- **F** toggles flying, **Escape** releases/captures the mouse
- **Space** jumps while grounded; **F5** switches first/third person

The texture atlas is generated in memory, so no external asset files are required.

The HUD shows your coordinates, flying state, targeted block, time of day, and nine-slot hotbar. Pressing **E** opens a 27-slot backpack and an inexhaustible palette of every placeable block, including oak-green leaves; movement, mining, and placement pause while it is open. The deterministic flat world includes sparse 4–6-block oak trunks with full leaf canopies that continue across chunk boundaries. Broken blocks fill the hotbar first and then overflow into the backpack. A procedural 12-minute day-night cycle changes the sky and fog while dimming terrain and the player together; propagated skylight also darkens holes, overhangs, and covered blocks.

World edits and the complete inventory state are loaded at startup and saved when the game exits. The single local save is `%APPDATA%\MinecraftClone\save.dat` on Windows or `~/.minecraftclone/save.dat` on Linux and macOS. A missing, unsupported, or corrupted save is ignored and starts a fresh world.
