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

- [ ] `F5` switches between first and third person.
- [ ] First person does not render the body into the camera.
- [ ] Third person shows the complete model from an offset angle.
- [ ] WASD, sprint, jump, flight, collision, and holes still work.
- [ ] Block targeting, breaking, placement, HUD, fog, and world rendering still work.

## Sign-off

- Date:
- Evaluator:
- Commit:
- Result:
- Notes:

Do not mark visual checks complete solely because the project builds or an automated test passes.
