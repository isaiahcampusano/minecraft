# Minecraft Superflat

A small Minecraft-style voxel demo in C++17 and OpenGL 3.3. It streams 16×256×16 chunks over a finite 1000×1000 world, generates a Bedrock + 3 Dirt + Grass superflat stack, meshes visible faces, and supports first-person movement, collision, and block editing.

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
- **F** toggles flying, **Escape** releases/captures the mouse
- **Space** jumps while grounded; **F5** switches first/third person

The texture atlas is generated in memory, so no external asset files are required.

The HUD shows your coordinates, flying state, targeted block, time of day, and nine-slot hotbar. A procedural 12-minute day-night cycle changes the sky and fog while dimming terrain and the player together; propagated skylight also darkens holes, overhangs, and covered blocks. Broken blocks go directly into the hotbar; right click places and consumes the selected item. A center crosshair and wireframe selection box make block editing precise; fog, a sky gradient, and textured block borders improve depth perception.
