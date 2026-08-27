# Player System

The player is a physical entity whose position is measured at its feet. It has a 0.6×1.8×0.6 block AABB and an eye height of 1.62 blocks.

## Components

- `Player` owns position, velocity, orientation, grounded state, and flight state.
- `PlayerPhysics` applies gravity and resolves movement one axis at a time against solid world blocks.
- `PlayerCamera` produces first-person and third-person views.
- `PlayerRenderer` draws a six-part, colored block character in third person.
- `PlayerAnimator` drives independent joint poses from velocity, grounded state, and flight state.
- `CuboidMesh` provides indexed six-face geometry, normals, and directional face colors.

## Inventory

`Inventory` owns a nine-slot hotbar, a 27-slot backpack, and the stack held by the inventory cursor. Breaking a non-bedrock block fills matching or empty hotbar stacks first, then overflows into the backpack. Stacks hold up to 64 blocks. Number keys 1–9 select the active hotbar slot, and placing consumes one block from that stack. The hotbar remains visible at the bottom center of the HUD.

## Controls

- **WASD:** move
- **Space:** jump; fly upward while flying
- **Left Shift:** fly downward
- **Left Control:** sprint
- **F:** toggle flight
- **F5:** switch first/third person
- **Mouse:** look/orbit
- **E:** open/close inventory

Gravity, landing, jumping, falling into holes, collision sliding, flight, and player-safe block placement all operate on the same player AABB.

The third-person model uses shoulder, hip, and neck pivots. Walking and sprinting swing opposing limbs, idle motion settles smoothly, and jumping, falling, and flying use distinct poses. The body smoothly faces movement while the constrained head tracks camera yaw and pitch. Facial details are offset slightly from the head surface to avoid z-fighting.

## v1.5

The inventory screen releases the cursor and suspends movement, mining, and placement while open. Its procedural UI shows the three-row backpack above the hotbar and a paged creative palette containing every non-air block registered by `BlockType`. Left-clicking a storage slot swaps it with the held cursor stack; clicking a palette entry creates a fresh 64-block stack without depleting the palette.

World edits are recorded in a sparse in-memory overlay and re-applied after streamed chunks regenerate, so placed and broken blocks survive any number of unload/reload cycles during the current run.
