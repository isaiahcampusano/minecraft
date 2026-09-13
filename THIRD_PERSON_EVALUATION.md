# Third-Person Animation Evaluation

Use this checklist after changes to the player renderer, animator, camera, or physics. Automated checks establish the mathematical behavior; visual items still require an interactive run because they concern perception and GPU output.

## Automated gates

Run:

```sh
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

The tests verify:

- Opposing arm/leg walking motion and visible silhouette movement.
- Jump, fall, flight, and smooth idle-return poses.
- Horizontal head tracking and the head-yaw limit.
- Gravity, landing, jumping, holes, collision, and gravity-free flight.

## Visual appearance

- [ ] All six faces remain visible from front, side, back, above, and below.
- [ ] Directional lighting and per-face colors make every cuboid read as 3D.
- [ ] Eyes and mouth appear on the face without flicker or overlap.
- [ ] Back-face culling produces no holes or inside-out faces.

## Joints and animation

- [ ] Head rotation keeps the neck attached.
- [ ] Arms rotate at the shoulders and legs rotate at the hips.
- [ ] Walking uses opposing arms and legs.
- [ ] Sprinting produces faster, wider motion than walking.
- [ ] Idle motion is subtle and limbs return smoothly to neutral.
- [ ] Jumping, falling, and flying have visibly distinct poses.
- [ ] The body smoothly faces movement while the head tracks the view.
- [ ] Head yaw and pitch stay within natural limits.

## Camera and regression checks

- [ ] `F5` cycles first person -> third person (back) -> third person (front) -> first person.
- [ ] First person does not render the body into the camera.
- [ ] Third-person back shows the complete model from an offset angle behind the player.
- [ ] Third-person front shows the complete model from in front, framed dead-center ("selfie" view), with the player still facing away from this camera.
- [ ] Camera does not render inside or through solid geometry in either third-person mode, whether the player is against a wall, under a low ceiling, or standing at map edges.
- [ ] The crosshair-targeted block (outline box, crack overlay, break/place result) matches the player's actual look direction in all three POV modes — including third-person front, where the drawn camera itself faces backward relative to that direction.
- [ ] Mob attacks in third-person front hit what the player is actually facing, not whatever is behind the player toward the camera.
- [ ] WASD, sprint, jump, flight, collision, and holes still work, and movement stays relative to the player's own facing (mouse-controlled) in all three POV modes rather than to the on-screen camera.
- [ ] Block targeting, breaking, placement, HUD, fog, and world rendering still work.

## Sign-off

- Date:
- Evaluator:
- Commit:
- Result:
- Notes:

Do not mark visual checks complete solely because the project builds or an automated test passes.
