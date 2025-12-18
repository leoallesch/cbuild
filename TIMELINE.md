# Build System Project — Deliverables Timeline

This document defines a **bottom‑up implementation plan** for a modern, C‑based build system with a `build_config.c` entry point, platform abstractions, custom allocators, and strong incremental‑build support.

The structure is designed to be dropped directly into a **GitHub Project board** or issue tracker.

---

## PHASE 0 — Project Foundation (Week 0–1)

### Goal

Establish a runnable, testable, open‑source project with clear structure.

### Deliverables

* Repository scaffold created
* Open‑source license added (MIT or Apache‑2.0)
* CI pipeline (Linux initially)
* CppUTest wired and runnable
* `build/` output directory standardized
* `.clang-format` and `.editorconfig`

### Artifacts

```
/src
/tests
/tools
/build
/.github/workflows
```

---

## PHASE 1 — Core Runtime Layer (Week 1–2)

### Goal

Foundational systems used by every other module.

### 1. Memory System

**Deliverables**

* `i_allocator.h` interface
* malloc‑backed allocator
* arena allocator (fixed + growable)
* scratch / temp allocator
* allocator unit tests (CppUTest)
* optional debug hooks (stats, leak tracking)

---

### 2. String & Path Utilities

**Deliverables**

* `string_view` type
* string builder
* path normalization (POSIX + Windows)
* string interner (hash‑table backed)
* optional UTF‑8 helpers

---

### 3. Logging & Diagnostics

**Deliverables**

* logger API
* log levels
* structured diagnostics
* colored console output (platform aware)
* test log capture

---

## PHASE 2 — Core Data Structures (Week 2–3)

### Goal

High‑performance internal storage.

### 4. Hash Table

**Deliverables**

* open‑addressed hashmap
* tombstone handling
* resizing / rehashing
* custom allocator support
* string‑key specialization
* unit tests

---

### 5. Graph / DAG Utilities

**Deliverables**

* DAG node abstraction
* adjacency lists
* cycle detection
* topological sort
* parallel‑ready traversal
* unit tests

---

## PHASE 3 — Platform Abstraction Layer (Week 3–4)

### Goal

Make the build system portable.

### 6. Filesystem API

**Deliverables**

* directory traversal
* file stat
* mkdir / remove
* atomic file replace
* canonical path resolution
* file watching stub (future)

---

### 7. Process Execution API

**Deliverables**

* spawn external processes
* capture stdout / stderr
* environment overrides
* working directory control
* parallel‑safe execution

---

## PHASE 4 — Dependency & Incremental Build (Week 4–5)

### Goal

Correct and fast incremental builds.

### 8. Dependency Scanner

**Deliverables**

* `.d` file parser (`-MMD`)
* dependency graph builder
* reverse dependency index
* header invalidation logic

---

### 9. File Hashing Engine

**Deliverables**

* content hashing (xxhash / sha1)
* hash cache
* timestamp vs content comparison
* large‑file optimization

---

## PHASE 5 — Build Graph & Rules (Week 5–6)

### Goal

Translate `build_config.c` into an executable DAG.

### 10. Target Model

**Deliverables**

* target types (exe, static, shared)
* public vs private dependencies
* mixed language support (C / C++ / ASM)
* compile & link rule definitions
* object file layout rules

---

### 11. DAG Construction

**Deliverables**

* per‑file compile nodes
* per‑target link nodes
* shared object reuse
* cycle diagnostics

---

## PHASE 6 — Execution Engine (Week 6–7)

### Goal

Fast, parallel builds.

### 12. Build Scheduler

**Deliverables**

* worker thread pool
* dependency‑aware scheduling
* failure propagation
* progress reporting

---

### 13. Build Cache

**Deliverables**

* rule hashing
* artifact storage
* local cache backend
* cache hit / miss metrics

---

## PHASE 7 — Testing & Tooling (Week 7–8)

### Goal

Strong developer experience.

### 14. Test Runner Integration

**Deliverables**

* test targets
* test discovery
* test execution rules
* junit / json output

---

### 15. Compilation Database

**Deliverables**

* `compile_commands.json` generation
* per‑file flag emission
* clangd compatibility

---

## PHASE 8 — External Libraries (Week 8–9)

### Goal

Composable, reusable projects.

### 16. External Projects

**Deliverables**

* subproject API
* configure / build hooks
* toolchain overrides
* vendored vs system libraries
* reproducible builds

---

## PHASE 9 — Polishing & Release (Week 9–10)

### Goal

Public‑ready open‑source release.

### 17. Documentation

**Deliverables**

* architecture overview
* `build_config.c` guide
* examples
* CONTRIBUTING guide

---

### 18. Release

**Deliverables**

* tagged `v0.1`
* changelog
* public roadmap

---

## Suggested First Issues

1. Arena allocator implementation
2. Hash table with tombstones
3. String interner

---

## License Recommendation

* **MIT** — simplest, permissive
* **Apache‑2.0** — includes patent protection (recommended for tooling)

Either is appropriate for open‑source distribution.
