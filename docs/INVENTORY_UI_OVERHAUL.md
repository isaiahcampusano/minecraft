# Inventory UI overhaul

The inventory uses one interaction model for the player inventory, crafting
grids, creative entries, and furnace slots. A slot is identified by its area
and index at the time of an operation; no UI operation retains a pointer to a
slot across frames. Furnace operations revalidate the active block before
mutating its state, and closing the screen cancels transient drag state.

The cursor stack remains part of `Inventory` and the MCv8 save format, but it
is not rendered as a floating HELD slot. It follows the mouse only while the
inventory is open, is drawn last, and suppresses tooltips while carrying.
Player inventory drag, right-click splitting, painting, and crafting output
behavior are preserved across personal and table crafting contexts.

Gameplay controls continue to come from `SettingsState`/`InputResolver`;
physical Shift is reserved for inventory quick-move precedence so remapping a
gameplay action cannot change inventory routing.
