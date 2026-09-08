# Minecraft Superflat

<img width="1281" height="716" alt="image" src="https://github.com/user-attachments/assets/f2dd7b8b-62ca-4aa1-8367-9cf5aea5df74" />


A small Minecraft-style voxel demo in C++17 and OpenGL 3.3. It streams 16×256×16 chunks over a finite 1000×1000 world, generates a varied Bedrock + Stone + Dirt superflat stack with full oak trees, meshes visible faces, and supports first-person movement, collision, inventory management, and persistent block editing.

## Build

Dependencies are downloaded by CMake during configuration.

```sh
cmake -S . -B build
cmake --build build --config Release
```

Run `Minecraft` from the build output directory.

## Controls

- **WASD** moves; press **Left Shift** to toggle sprint, or double-tap **W** within 300 ms. Sprinting requires more than six hunger points
- **Space** jumps or flies upward; **Left Control** flies downward
- **Mouse** look, **scroll** changes field of view
- **Left click** attacks a closer mob within 4.5 blocks; otherwise, hold it to mine the targeted block. Harder materials take longer, and by default one press breaks one block
- **Right click** places the selected hotbar block on the selected face; hold it for 1.6 seconds with an apple selected to eat
- **1–9** selects a hotbar slot
- **E** opens the backpack, 2×2 personal crafting grid, and creative block palette; click slots to swap stacks and scroll to change palette pages
- **Right click a crafting table** opens its 3×3 crafting grid
- **G** toggles the HUD break mode between **SINGLE** and **FULL AUTO**; in full-auto, holding left click continues into the next targeted block after a short 120 ms cooldown
- **F** toggles flying, **Escape** releases/captures the mouse
- **Space** jumps while grounded; **F5** switches first/third person

The texture atlas is generated in memory, so no external asset files are required.

The HUD shows your coordinates, flying and sprint state, break mode, targeted block, time of day, ten hearts, ten hunger icons, and nine-slot hotbar. Pressing **E** opens a 27-slot backpack, personal crafting grid, and an inexhaustible item palette containing every placeable block, material, and food; movement, mining, placement, sprinting, and eating pause while it is open. Logs craft into planks, planks into sticks and crafting tables, and table recipes produce wooden and stone tools. Matching tools mine faster and lose durability; stone and cobblestone only drop when mined with a wood-or-better pickaxe. The deterministic flat world includes sparse 4–6-block oak trunks with full leaf canopies that continue across chunk boundaries. Broken leaves never drop leaf blocks and have a 0.5% chance to create an apple world drop.

Cows, pigs, and sheep spawn naturally in small packs and persist with the world. Adults idle and wander, babies follow nearby adults, damaged animals flee, and sheep graze grass into dirt. Animals use terrain-aware movement and attack hitboxes, mature after 20 minutes, and drop species-specific raw food and materials when an adult is killed. Drops remain in the world until collected or despawned, including when the inventory is full.

Survival starts at 20 health, 20 hunger, and five hidden saturation points. Sprinting and jumping build exhaustion, which consumes saturation before hunger. Apples restore four hunger and 2.4 saturation. At 18 or more hunger, injured players naturally heal one health every four seconds; at zero hunger, starvation deals one damage every four seconds. Falls farther than three blocks deal damage. Death clears all carried and crafting-grid items, preserves world edits, and respawns the player at a safe open position near the world center. Sprinting smoothly widens the current user-selected FOV by 10 degrees, capped at 100 degrees.

A procedural 12-minute day-night cycle changes the sky and fog while dimming terrain and the player together; propagated skylight also darkens holes, overhangs, and covered blocks.

World edits, inventory, food stacks, survival state, and passive mobs are loaded at startup and saved when the game exits. New saves use the `MCv4` format. Existing `MCv2` and `MCv3` saves migrate automatically and are rewritten as `MCv4` on the next exit. The single local save is `%APPDATA%\MinecraftClone\save.dat` on Windows or `~/.minecraftclone/save.dat` on Linux and macOS. A missing, unsupported, or corrupted save is ignored and starts a fresh world.
