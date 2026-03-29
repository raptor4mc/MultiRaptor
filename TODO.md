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

### sub priority 4.1
Enforce declaration-before-assignment for all assignment forms.

### sub priority 4.2
Reject invalid control usage early (for example `return` outside functions).

### sub priority 4.3
Keep semantic diagnostics short, actionable, and line-oriented.

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

## priority 7: CLI and developer workflow quality
Status: 🔄 in progress.

### sub priority 7.1
Add machine-readable CLI output mode (JSON) for automation.

### sub priority 7.2
Add `--format-errors` option to normalize parse/semantic/runtime diagnostics.

### sub priority 7.3
Add `--run` workflow command for parse+check+execute in one step.

### sub priority 7.4
Add `--module-graph` command to print dependency graph edges.

### sub priority 7.5
Add CLI integration tests for exit codes and failure-path consistency.
