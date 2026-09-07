# Player System

The player is a physical entity whose position is measured at its feet. It has a 0.6×1.8×0.6 block AABB and an eye height of 1.62 blocks.

## Components

- `Player` owns position, velocity, orientation, grounded state, and flight state.
- `PlayerPhysics` applies gravity and resolves movement one axis at a time against solid world blocks.
- `PlayerCamera` produces first-person and third-person views.
- `PlayerRenderer` draws a six-part, colored block character in third person.
- `PlayerAnimator` drives independent joint poses from velocity, grounded state, and flight state.
- `SprintState` separates the player's persistent sprint request from whether sprint is currently allowed and active.
- `SurvivalState` owns health, hunger, saturation, exhaustion, healing/starvation timers, and fall tracking.
- `CuboidMesh` provides indexed six-face geometry, normals, and directional face colors.

## Inventory

`Inventory` owns a nine-slot hotbar, a 27-slot backpack, two crafting-input grids, and the stack held by the inventory cursor. Block, material, and food stacks hold up to 64 items. Number keys 1–9 select the active hotbar slot, and placing or eating consumes one item from that stack. The hotbar remains visible at the bottom center of the HUD.

## Controls

- **WASD:** move
- **Space:** jump; fly upward while flying
- **Left Shift:** toggle sprint while grounded
- **Double-tap W:** request sprint within a 300 ms window
- **Left Control:** fly downward
- **F:** toggle flight
- **F5:** switch first/third person
- **Mouse:** look/orbit
- **E:** open/close inventory

Gravity, landing, jumping, falling into holes, collision sliding, flight, and player-safe block placement all operate on the same player AABB.

Walking speed is 5 units/second and sprint speed is 10. Releasing W pauses active sprint without discarding the request, so moving forward again resumes it. Inventory, flight, eating, low hunger, death, and respawn cancel the request. Sprinting adds 0.1 exhaustion per horizontal unit traveled; normal and sprint jumps add 0.05 and 0.2 exhaustion at takeoff.

The player begins with 20 health, 20 hunger, and five saturation. Every four accumulated exhaustion consumes one saturation point, then one hunger point once saturation reaches zero; fractional exhaustion carries forward. Natural healing and starvation operate on discrete four-second timers. Airborne downward velocity accumulates fall distance, with damage beginning beyond three blocks. Entering flight clears fall distance. Death clears all inventory and crafting inputs before a safe-spawn search at the world center.

Apples restore four hunger and 2.4 saturation after a 1.6-second right-click hold. Normal grounded walking is permitted while eating; releasing right click, changing the selected item, opening inventory, losing capture, sprinting, jumping, falling, flying, taking damage, or dying cancels the action.

The third-person model uses shoulder, hip, and neck pivots. Walking and sprinting swing opposing limbs, idle motion settles smoothly, and jumping, falling, and flying use distinct poses. The body smoothly faces movement while the constrained head tracks camera yaw and pitch. Facial details are offset slightly from the head surface to avoid z-fighting.

## v1.5

The inventory screen releases the cursor and suspends movement, mining, and placement while open. Its procedural UI shows the three-row backpack above the hotbar and a paged creative palette containing every non-air block registered by `BlockType`. Left-clicking a storage slot swaps it with the held cursor stack; clicking a palette entry creates a fresh 64-block stack without depleting the palette.

World edits are recorded in a sparse in-memory overlay and re-applied after streamed chunks regenerate, so placed and broken blocks survive any number of unload/reload cycles during the current run.
