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
- **Left click** breaks a block
- **Right click** places dirt on the selected face
- **F** toggles flying, **Escape** releases/captures the mouse

The texture atlas is generated in memory, so no external asset files are required.

The HUD shows your coordinates, flying state, and targeted block. A center crosshair and wireframe selection box make block editing precise; fog, a sky gradient, and textured block borders improve depth perception.
