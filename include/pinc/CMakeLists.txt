set(LLVM_TARGET_DEFINITIONS pincPasses.td)
mlir_tablegen(pincPasses.h.inc --gen-pass-decls)
add_public_tablegen_target(MLIRpincPassesIncGen)
