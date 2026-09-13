# Main Menu, Mouse Controls, and Complete Keybindings

## Overview

The game starts at a title screen without opening a world. Choose **Singleplayer** to create or select a world, **Settings** to configure controls and video, or **Quit** to close the application. Audio and multiplayer are not included.

This implementation combines cooking commit `cd6feaa` with camera/main commit `1fb0c290`. Furnaces, cooking, all three camera perspectives, and interaction rays from the player's eyes are retained.

## Menus

- Click a button, or use Tab / Shift+Tab and Up / Down to move focus. Enter or Space activates the focused control.
- Sliders support dragging and Left / Right adjustment.
- World and keybinding lists support the wheel, Page Up / Page Down, and visible previous/next buttons.
- Escape goes back, cancels binding capture, closes an inventory or chat, or pauses/resumes a world. At the title screen it does nothing.
- Pause provides Resume, Settings, Keybinds, and Save & Quit. Settings returns to the screen it came from.
- Delete names the selected world and initially focuses Cancel. Names do not determine filesystem paths.
- Save & Quit saves successfully before unloading the session and returning to the title. A failed save leaves the session open and reports the error. Closing the application window during a session also requires a successful save.

Rendering and hit testing use the same widget rectangles. Menus use a 1280 x 720 logical canvas mapped to the window/framebuffer. Inventory hit testing converts window coordinates to framebuffer coordinates.

## Worlds, seeds, and storage

Create a world with a trimmed, printable ASCII name of 1–48 characters, an optional unsigned decimal seed from 0 through 4294967295, and Survival or Creative mode. Duplicate names are allowed. Blank seeds are randomly generated and shown after creation and in the world list.

Seed 0 preserves the legacy generator. Other seeds deterministically change surface patches and trees, including trees crossing chunk boundaries. Superflat layers and finite dimensions remain unchanged.

On Windows, storage is under `%APPDATA%/MinecraftClone`:

```text
settings.cfg
legacy-import.txt
save.dat                         # original legacy save, if present
worlds/
  <32-character-generated-id>/
    world.meta
    save.dat
```

On other platforms the root is `$HOME/.minecraftclone`, matching the existing save-path convention. Metadata uses `MCWorld 1`, with the quoted display name, seed, creation time, last-played time, and import flag. Times are Unix seconds; the list displays local time. The save is authoritative for seed and gameplay mode. Invalid metadata or saves remain visible with an error and cannot be played as fresh worlds.

The first successful legacy import validates and copies the old `save.dat` into **Imported World**, preserves the original bytes, and records the generated ID afterward. The completed import flag allows recovery if writing the import marker fails. Subsequent launches do not import again.

Deletion validates the generated ID and resolved path beneath the world root and rejects redirected world/save paths. It removes only that world's directory. Saves, metadata, import markers, and preferences use temporary-file replacement.

Loading establishes a fresh `WorldSession`; unloading clears chunks, edits, furnaces, mobs, inventories and both crafting grids, cursor items, particles, loose drops, chat, and transient actions. Pose, seed, time, mode, bed spawn, survival state, mobs, inventory, and furnace state are restored from the selected save. Invalid positions use the existing safe-spawn fallback with collision checks.

## Default controls

All 28 actions appear in Keybinds. Keyboard and GLFW mouse buttons are supported for gameplay actions.

| Action | Default |
|---|---|
| Move forward / backward / left / right | W / S / A / D |
| Jump / fly up | Space |
| Fly down | Left Ctrl |
| Sprint toggle; forward double tap also supported | Left Shift |
| Inventory | E |
| Toggle flight in Creative | F |
| Cycle first person, third-person back, third-person front | F5 |
| Hotbar slots 1–9 | 1–9 |
| Pause | Escape |
| Attack / break | Mouse Left |
| Use / place | Mouse Right |
| Pick block | Mouse Middle |
| Drop one item | Q |
| Sneak | Left Ctrl |
| Open local chat | T |
| Toggle HUD | F1 |
| Hold zoom | C |

The gameplay wheel cycles hotbar slots with wrapping. In a Creative inventory it changes catalog pages; in lists and chat it scrolls their contents.

Select a binding row and release the activating input, then press a keyboard key or mouse button. Escape cancels. Conflicts require a confirmed **Swap** naming both actions; Cancel changes neither. A swap that would create an invalid map is rejected. Multiple conflicts are listed and rejected. Per-row Reset follows these rules; Reset All restores the complete default binding map without changing video or mouse settings.

Sneak and Fly Down may share an input because Sneak applies when not flying and Fly Down applies when flying. Escape is reserved for Pause and always remains a pause/menu-back fallback, even if Pause is rebound.

Menu navigation and inventory left-clicking remain ordinary UI controls. If Inventory is assigned to Mouse Left, clicking an inventory slot operates that slot; clicking outside the slots closes the inventory. Escape also closes it. Closing an inventory retains cursor and furnace contents.

## Mouse and video settings

| Setting | Values | New-install default |
|---|---|---|
| View distance | 2–8 chunks | 4 |
| Base FOV | 30–90 degrees | 70 |
| VSync | On / Off | On |
| Mouse sensitivity | 0.01–1.00 degrees per input unit | 0.15 |
| Invert Y | On / Off | Off |
| Raw mouse | On / Off when GLFW supports it | On |

Changes apply immediately and preferences are saved. From the title, the mode control is **Default New World Mode**. While paused it changes the current world's mode using the existing transition logic.

Gameplay captures the cursor. Menus, inventories, and chat release it. Supported raw motion is enabled only while captured. Sensitivity is applied once to callback deltas; player look no longer applies another multiplier. Recapture resets the first motion sample. Focus loss pauses an active world and suppresses held actions. Inputs held across transitions must be released before they trigger gameplay again.

## Gameplay actions

**Pick Block** uses the player-eye ray in every perspective. It selects a matching hotbar stack first, otherwise swaps a matching backpack stack into the selected slot. Creative creates a full stack if no match exists; Survival does nothing without a matching inventory stack.

**Drop Item** creates one forward-thrown item before removing it from the selected stack. Tool kind, tier, and durability are preserved. Pickup is delayed 0.75 seconds. A full drop pool or unsafe spawn retains the item and shows feedback. Loose drops remain temporary and are cleared on unload.

**Sneak** moves at 30% walking speed and cancels sprinting. Grounded horizontal motion, including diagonal motion, is limited to retain support beneath the player. Jumping and airborne motion are not protected. Player size and camera height are unchanged; while flying Ctrl controls descent.

**Toggle HUD** hides the crosshair, hotbar, survival meters, and diagnostics. It does not hide menus, inventories, or chat. **Zoom** smoothly targets 35% of base FOV while held, suppresses the sprint FOV bonus, and restores the current normal/sprint FOV on release. It does not modify the saved FOV or interaction reach.

Furnace, bed, crafting-table, food-use, and placement precedence is preserved. Attack and Use holds, including mining and eating, resolve their configured input rather than hardcoded mouse buttons.

## Local chat and commands

Chat accepts up to 256 printable ASCII characters with a caret, Left / Right, Home / End, Backspace, and Delete. Enter submits; Escape cancels. Up / Down recalls up to 20 commands. Up to 100 messages are retained (long messages wrap); the wheel scrolls history. There is no network transmission or persistent transcript.

| Command | Result |
|---|---|
| `/help` | Show available commands |
| `/gamemode survival` or `/gamemode creative` | Change the active mode |
| `/time set day` | Set morning, time 0.25 |
| `/time set night` | Set night, time 0.75 |
| `/tp <x> <y> <z>` | Teleport to a finite, in-bounds, collision-free position |

Teleport loads destination chunks for collision/headroom checks and rejects unsafe destinations. Successful teleport clears velocity, fall accumulation, mining and ongoing actions, without changing the bed respawn point. Unknown commands, extra arguments, and invalid values report errors without changing gameplay state. Commands are available in either mode.

Chat suppresses camera motion and gameplay bindings while the world continues running. Ordinary inventories and furnace screens also allow simulation and cooking. Pause/settings freeze simulation. Setting time and sleeping never fast-forward furnace timers. There is no offline catch-up.

## Compatibility and implementation

World saves advance from MCv7 to **MCv8**, appending the uint32 seed, player position, yaw/pitch, and day/night time after the existing furnace payload. MCv2–MCv7 readers remain supported and default to seed 0, starting pose, and morning. Existing block, food, and item IDs are unchanged. Malformed records fail before applying the save.

Preferences advance from `MCSettings 1` to **MCSettings 2**, preserving stable action names. Old settings migrate with sensitivity **0.10** to preserve mouse feel. Existing bindings are retained; new actions receive defaults or unused alternatives when needed. A migration notice is displayed.

Primary responsibilities:

- `MenuController` / `MenuView`: shared widget geometry, focus, hit testing and text editing; Application dispatches menu actions and dialogs.
- `InputResolver`: contextual keyboard/mouse held, pressed, released and transition suppression.
- `WorldRepository`: named-world listing, validation, creation, import, persistence and deletion.
- `WorldSession`: owned active gameplay state with Application's explicit enter/unload boundaries.

## Verification

Build with CMake and run `ctest --test-dir build --output-on-failure`. The suite includes legacy terrain fixtures and physics/camera tests, cooking and save regressions, settings migration and input tests, repository and deletion tests, seeded chunk reloads, and sneak edge checks.

`MenusApplicationTests` exercises title startup, named-world creation and isolation, capture activation/release, Escape cancellation, confirmed swaps, remapped movement/Use/Attack, camera sensitivity, pick/drop, tool-drop durability and capacity, zoom, chat suppression and commands, furnace pause/chat timing, save-failure recovery, and safe deletion. It renders menu screens and all three camera perspectives at 1280 x 720 and 2560 x 1440 using actual OpenGL framebuffers. `FurnaceRenderTests` checks lit/unlit furnace scenes and both inventory modes at those scales. Graphical tests return skip code 77 if a graphics context cannot be created.

To export application review images: `MenusApplicationTests <output-directory>`. Full interactive playtesting remains useful for platform-specific mouse/display behavior.
