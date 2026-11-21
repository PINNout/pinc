#ifndef PINC_PASS
#define PINC_PASS

include "mlir/Pass/PassBase.td"

def PincRedMulE: Pass<"redmule", "::mlir::ModuleOp"> {
  let summary = "Offloads matmuls to RedMulE.";
  let description = [{
    "offload all matmuls to RedMulE. Smarter mapping is WIP"
  }];

  let dependentDialects = [
    "mlir::arith::ArithDialect",
    "mlir::memref::MemRefDialect",
    "mlir::emitc::EmitCDialect"
    ];
} 

#endif // PINC_PASS
