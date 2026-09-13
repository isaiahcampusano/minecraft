# Cooking v2.1

Craft a furnace at a crafting table using eight cobblestone in a ring with an empty center. Place it and right-click to open it. The top slot accepts raw beef, raw porkchop, or raw mutton; the lower slot accepts planks, oak logs, or sticks. The right slot contains cooked food and only allows extraction.

Click to pick up or place stacks, or drag between slots. Matching stacks merge up to 64. Hover for item names. Escape or your Inventory binding closes the screen without stopping cooking or losing a held stack.

Each meat cooks in 10 seconds. Logs and planks burn for 15 seconds; sticks burn for 5 seconds. The arrow shows cooking progress and the flame bar shows remaining fuel. Burning furnaces have orange openings; unlit furnaces have dark openings.

Fuel already burning continues to expire if input is removed or output is blocked. New fuel is consumed only when cooking can proceed. Running out of fuel or blocking output preserves progress. Removing input or changing its food type resets progress.

Furnaces run only while their chunks are loaded. Unloading, pause/settings menus, and exiting the game freeze their timers. Inventory screens do not pause cooking. Sleeping does not accelerate it. Creative uses the same cooking time and fuel costs as Survival.

Breaking a furnace drops its remaining input, unused fuel, and output. Burning fuel and partial progress are lost. In Survival, recovering the furnace block requires a wooden-or-better pickaxe; Creative does not drop the block itself.

Cooked beef and pork restore 8 hunger and 4.8 saturation; cooked mutton restores 6 hunger and 3.6 saturation.

## Persistence

MCv7 stores furnace contents and timers after the MCv6 payload. MCv2 through MCv6 remain readable and start with no furnace state. Existing item IDs, world edits, game mode, mobs, survival stats, and bed spawn coordinates are preserved. Settings remain in settings.cfg.

No ores, charcoal, experience, automation, directional placement, emitted lighting, or offline cooking are added.

## Verification

Build and run the existing CTest suite normally. FurnaceTickTests covers recipes, fuel timing, transfers, nutrition, and layout. FurnacePersistenceTests covers chunk lifecycle, drops, validation, and MCv2-MCv6 migration. GameStateTests also round-trips an unloaded active furnace.

FurnaceRenderTests renders lit/unlit blocks and both inventory modes at 1280x720 and 2560x1440 using offscreen framebuffers. Pass an output directory to save PPM captures for visual review. CookingApplicationTests exercises right-click opening, slot input, inventory remapping, paused simulation, output extraction, closing/reopening, restart, and invalidation through the application handlers with an isolated temporary save. Graphical tests skip with code 77 if OpenGL is unavailable.
