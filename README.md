# pinc – Physics-Informed Neural Network Compiler

pinc is an experimental set of MLIR passes for compiling physics-informed
neural networks (PINNs) onto [MAGIA](https://github.com/pulp-platform/MAGIA),
a tile-based RISC-V accelerator platform. It is developed as part of a
master's thesis at KU Leuven.

A PINN defined in JAX is exported as StableHLO and lowered to C kernels for
the MAGIA SDK. Most of that lowering uses existing StableHLO and upstream MLIR
passes. pinc adds custom passes that map operations onto MAGIA's hardware. The
surrounding toolchain, which runs the full flow in the GVSoC simulator, lives
in [pinnout-workbench](https://github.com/PINNout/pinnout-workbench).

The goal is to explore how PINN workloads can be mapped onto architectures of
this kind, starting with a Poisson example.

## Status

Work in progress. Currently implemented:

- `pinc-redmule`: offloads 16x16 f16 `linalg.matmul` ops to the RedMulE matrix
  accelerator.
- `pinc-memref-to-emitc`: converts memref pointer extraction to EmitC.

Planned: memory placement, scheduling, choice of functional unit, and a study of
reduced numerical precision.
