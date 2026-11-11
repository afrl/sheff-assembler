# SHEFF Assembler (`sheff-assembler`)

`sheff-assembler` assembles element-level tangent stiffness matrices, residual vectors, and right-hand-side force contributions into the global sparse linear system in the **Scalable Hierarchical Enrichment Finite-element Framework (SHEFF)**.

## Capabilities

- **Global Sparsity Pattern Construction (`Assembler.h`, `SerialAssembler.h`)**:
  - Builds global CSR non-zero sparsity graphs directly from [`sheff-dof-manager`](https://github.com/afrl/sheff-dof-manager) element scatter maps, supporting variable DoF counts per element arising from hierarchical XFEM enrichments.
- **Global Matrix & Residual Assembly**:
  - Evaluates element stiffness matrices and internal/external force vectors via [`sheff-finite-element`](https://github.com/afrl/sheff-finite-element) and scatters them into backend-agnostic [`sheff-sparse-algebra`](https://github.com/afrl/sheff-sparse-algebra) matrices and vectors, as well as extracting element-level solution vectors from global state vectors.
- **Preallocated Workspace Management (`ThreadWorkspace.h`)**:
  - Reuses preallocated per-thread element workspaces across assembly loops to avoid heap allocations inside inner quadrature loops.

## Project Structure

- `include/assembler/`: Public headers (`Assembler.h`, `SerialAssembler.h`, `ThreadWorkspace.h`).
- `source/`: Sparsity graph builders and global matrix/vector assembly routines.
- `unit_tests/`: Unit tests verifying sparsity allocation and global system assembly on standard and enriched meshes.
- **CMake Package / Target**: `Assembler` / `Assembler::assembler` (static library).

## Dependencies

- **Build System**: [`sheff-build-tools`](https://github.com/afrl/sheff-build-tools) (`SHEFF_TOOLS`), CMake 3.17+, C++17/C++20 compiler.
- **First-Party SHEFF Modules**: [`sheff-dof-manager`](https://github.com/afrl/sheff-dof-manager) (`DofManager::dof_manager`), [`sheff-sparse-algebra`](https://github.com/afrl/sheff-sparse-algebra) (`SparseLinearAlgebra::sparse_linear_algebra`).

## Building

When building as part of [SPAWC](https://github.com/afrl/spawc), `sheff-assembler` is configured and built automatically via `make.py`.

To build and test standalone (with `SHEFF_TOOLS` configured and a sparse linear algebra backend such as `--use-mkl` enabled):

```bash
python3 make.py --config --build --use-mkl --make-tests --run-tests
```

## Public Release Information

Distribution Statement A. Approved for public release: distribution is unlimited. Case #: AFRL-2026-4513
