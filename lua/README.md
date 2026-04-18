# Lua integration boundary

This directory is reserved for sandboxed Lua scripting, plugin loading, and extension APIs.

Lua is intentionally **not**:
- part of the MagPhos language design,
- part of `Lib/` standard library,
- a fallback execution engine.

Keep Lua isolated from `src/` core runtime internals and from the MagPhos language surface.
