## TODO rules (read first)
1. Keep this rules section at the top of the file.
2. For every newly implemented feature, add one new `priority` section with exactly five `sub priority` items.
3. When a feature/sub-priority is fully implemented, remove it from the active TODO list (or mark the whole priority completed and move on).
4. Always work on the first `priority` that is not completed.
5. Each `sub priority` must be implementation-oriented and directly testable.
6. No bloat priorities: every priority/sub-priority must fix a real error or move the language/runtime/tooling forward.
7. If a `sub priority` is completed, remove it from active list and add one new `priority` with five concrete `sub priority` items.
8. If a full `priority` is completed, add five new non-overlapping priorities (merge near-duplicates).

---

## priority 1: folder-aware project management in Web Studio
Status: ✅ completed in current milestone.

### sub priority 1.1
Add a dedicated UI control to create folder paths (example: `src/utils`).

### sub priority 1.2
Normalize file/folder separators to `/` for cross-platform exports.

### sub priority 1.3
Auto-create parent folders when users add/rename files into nested paths.

### sub priority 1.4
Render folders distinctly from files in the sidebar (icon + muted style).

### sub priority 1.5
Preserve folder metadata in import/export/localStorage restore.

Completion note:
- Implemented all sub priorities 1.1–1.5 in `web/playground.js` + `web/playground.css`.

---

## priority 2: C++-first compiler pipeline for web
Status: ✅ completed in current milestone.

### sub priority 2.1
Extract shared compile logic into reusable C++ API for CLI + WASM.

### sub priority 2.2
Expose stable WASM compile entrypoint (`compileMagPhos`).

### sub priority 2.3
Configure WASM target with web-friendly flags (ES module + modularized loader).

### sub priority 2.4
Load WASM from playground and retire handwritten JS compiler fallback path.

### sub priority 2.5
Show actionable runtime guidance when WASM artifacts are missing.

---

## priority 3: release hardening for ongoing updates
Status: ✅ completed for process baseline; continue per-release.

### sub priority 3.1
Append at least three new high-level priorities after each milestone.

### sub priority 3.2
Define at least five concrete sub-priorities per high-level priority.

### sub priority 3.3
Keep TODO wording implementation-oriented with verb-first action statements.

### sub priority 3.4
Align TODO updates with real code changes from the same commit.

### sub priority 3.5
Enforce consistent TODO structure and spelling (`priority` / `sub priority`).

---

## priority 4: language safety and predictability
Status: 🔄 in progress.

### sub priority 4.4
Guarantee runtime errors map to stable error codes/messages.

### sub priority 4.5
Add regression tests for every safety rule added to parser/semantic/runtime.

---

## priority 5: module/package scalability
Status: 🔄 in progress.

### sub priority 5.1
Stabilize namespace/package syntax and document supported forms.

### sub priority 5.2
Add module cache invalidation strategy for long-running sessions.

### sub priority 5.3
Add duplicate-import deduping at dependency collection stage.

### sub priority 5.4
Add cycle-report diagnostics that show the import chain path.

### sub priority 5.5
Add integration tests for nested package layouts and mixed `import`/`use`.

---

## priority 6: function expressiveness and composability
Status: 🔄 in progress.

### sub priority 6.1
Stabilize default parameter evaluation order and side-effect behavior.

### sub priority 6.2
Stabilize variadic argument semantics and array representation contract.

### sub priority 6.3
Add higher-order function support for passing/storing callable values.

### sub priority 6.4
Add closure capture semantics for lexical environments.

### sub priority 6.5
Add semantic checks and tests for arity/default/variadic combinations.

---

## priority 8: nested tree navigation quality (added after completing 1.1)
Status: 🔄 in progress.

### sub priority 8.1
Add collapsible folder nodes in sidebar tree.

### sub priority 8.2
Persist folder expand/collapse state in localStorage.

### sub priority 8.3
Highlight active file ancestry folders.

### sub priority 8.4
Auto-scroll sidebar to active file on switch.

### sub priority 8.5
Add keyboard navigation for tree (up/down/left/right).

---

## priority 9: path validation hardening (added after completing 1.2)
Status: 🔄 in progress.

### sub priority 9.1
Reject illegal path segments (`..`, empty segments, control chars).

### sub priority 9.2
Normalize duplicate separators (`foo//bar` -> `foo/bar`).

### sub priority 9.3
Reject reserved names and invalid file extensions where needed.

### sub priority 9.4
Surface inline validation errors instead of alerts.

### sub priority 9.5
Add path normalization test fixtures for import/export roundtrip.

---

## priority 10: folder lifecycle management (added after completing 1.3)
Status: 🔄 in progress.

### sub priority 10.1
Add folder rename with automatic child path updates.

### sub priority 10.2
Add folder delete with safe confirmation and file impact preview.

### sub priority 10.3
Prune empty auto-created folders when files move/delete.

### sub priority 10.4
Support drag-and-drop move for files/folders.

### sub priority 10.5
Add undo support for move/rename/delete operations.

---

## priority 11: sidebar clarity and accessibility (added after completing 1.4)
Status: 🔄 in progress.

### sub priority 11.1
Improve visual distinction for folder vs file states (open/closed/current).

### sub priority 11.2
Add ARIA labels/roles for folder tree elements.

### sub priority 11.3
Ensure sufficient color contrast for muted folder styling.

### sub priority 11.4
Add empty-folder placeholder text for discoverability.

### sub priority 11.5
Add screen-reader announcements for create/rename/delete actions.

---

## priority 12: project metadata schema stability (added after completing 1.5)
Status: 🔄 in progress.

### sub priority 12.1
Version project JSON schema and add migrators per version.

### sub priority 12.2
Include optional folder metadata (`createdAt`, `tags`, `notes`) safely.

### sub priority 12.3
Add import warning for unknown metadata keys.

### sub priority 12.4
Add export option for minified vs pretty JSON.

### sub priority 12.5
Add schema validation tests for backward compatibility.

---

## priority 13: CLI JSON contract stabilization (added after CLI feature rollout)
Status: 🔄 in progress.

### sub priority 13.1
Add deterministic ordering guarantees for JSON arrays (`deps`, `tokens`, `moduleGraph`) where applicable.

### sub priority 13.2
Add explicit JSON schema version key to every CLI JSON response.

### sub priority 13.3
Add `stderr` field support in JSON responses for runtime failures.

### sub priority 13.4
Add golden-file tests for `--check/--deps/--tokens/--run/--module-graph --json`.

### sub priority 13.5
Document the CLI JSON response contract in `docs/` with stability guarantees.

---

## priority 14: control-flow diagnostics hardening
Status: 🔄 in progress.

### sub priority 14.1
Reject `return` outside function scope with stable diagnostic wording.

### sub priority 14.2
Add regression tests for invalid control flow in top-level and block scopes.

### sub priority 14.3
Normalize control-flow diagnostic prefixes for machine filtering.

### sub priority 14.4
Ensure CLI `--check --json` includes control-flow diagnostics in `errors[]`.

### sub priority 14.5
Document control-flow semantic rules in `docs/syntax.md`.

---

## priority 15: semantic diagnostic contract stabilization
Status: 🔄 in progress.

### sub priority 15.1
Guarantee semantic diagnostics render as one-line entries (no embedded newlines per issue).

### sub priority 15.2
Add deterministic ordering for semantic issues emitted from a single file.

### sub priority 15.3
Add machine-stable semantic error code prefixes (`SEM###`) for CLI/IDE filtering.

### sub priority 15.4
Expose semantic issue count in CLI JSON responses for `--check`.

### sub priority 15.5
Document semantic diagnostic format and compatibility policy in `docs/error_guide.md`.
