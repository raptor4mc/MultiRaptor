## priority 1: folder-aware project management in Web Studio
Enable explicit folder creation and nested path handling so projects stay organized as they grow.
### sub priority 1.1
Add a dedicated UI control to create folder paths (for example `src/utils`).
### sub priority 1.2
Normalize file and folder separators to `/` to keep exports cross-platform.
### sub priority 1.3
Auto-create parent folders when users add or rename files into nested paths.
### sub priority 1.4
Show folders distinctly from files in the project sidebar (folder icon + muted style).
### sub priority 1.5
Preserve folder metadata during import/export/localStorage restore.

## priority 2: C++-first compiler pipeline for web
Keep one source of truth by using the C++ compiler for both native and browser builds through WASM.
### sub priority 2.1
Extract shared compile logic into a reusable C++ API callable by both CLI and WASM.
### sub priority 2.2
Expose a stable WASM function (`compileMagPhos`) via Emscripten bindings.
### sub priority 2.3
Update CMake WASM target with web-friendly link flags (ES module + modularized loader).
### sub priority 2.4
Load the WASM module from the playground and remove the hand-written JS compiler path.
### sub priority 2.5
Show clear runtime guidance when WASM artifacts are missing.

## priority 3: release hardening for ongoing updates
After each code update, expand planning details so priorities stay actionable and trackable.
### sub priority 3.1
Append at least three new high-level priorities reflecting the latest milestone state.
### sub priority 3.2
For each new priority, add at least five concrete sub-priorities with owner/action language.
### sub priority 3.3
Keep TODO wording implementation-oriented (verb-first) rather than placeholder text.
### sub priority 3.4
Align TODO updates with real code changes from the same commit.
### sub priority 3.5
Review TODO structure for spelling/consistency (`priority` + numbered `sub priority`).
