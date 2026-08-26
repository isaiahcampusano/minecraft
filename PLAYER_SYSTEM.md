# Player System

The player is a physical entity whose position is measured at its feet. It has a 0.6×1.8×0.6 block AABB and an eye height of 1.62 blocks.

## Components

- `Player` owns position, velocity, orientation, grounded state, and flight state.
- `PlayerPhysics` applies gravity and resolves movement one axis at a time against solid world blocks.
- `PlayerCamera` produces first-person and third-person views.
- `PlayerRenderer` draws a six-part, colored block character in third person.
- `PlayerAnimator` drives independent joint poses from velocity, grounded state, and flight state.
- `CuboidMesh` provides indexed six-face geometry, normals, and directional face colors.

## Controls

- **WASD:** move
- **Space:** jump; fly upward while flying
- **Left Shift:** fly downward
- **Left Control:** sprint
- **F:** toggle flight
- **F5:** switch first/third person
- **Mouse:** look/orbit

Gravity, landing, jumping, falling into holes, collision sliding, flight, and player-safe block placement all operate on the same player AABB.

The third-person model uses shoulder, hip, and neck pivots. Walking and sprinting swing opposing limbs, idle motion settles smoothly, and jumping, falling, and flying use distinct poses. Facial details are offset slightly from the head surface to avoid z-fighting.
