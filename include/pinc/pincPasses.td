#ifndef PINC_PASS
#define PINC_PASS

include "mlir/Pass/PassBase.td"

def PincRedMulE: Pass<"pinc-redmule", "::mlir::ModuleOp"> {
  let summary = "Offloads matmuls to RedMulE.";
  let description = [{
    offload all matmuls to RedMulE. Smarter mapping is WIP
  }];

  let dependentDialects = [
    "mlir::arith::ArithDialect",
    "mlir::memref::MemRefDialect",
    "mlir::emitc::EmitCDialect"
  ];
}

def PincMemRefToEmitC: Pass<"pinc-memref-to-emitc", "::mlir::ModuleOp"> {
  let summary = "Converts memref array pointers to emitc pointers";
  let description = [{
    Converts memref array pointers to emitc pointers
  }];

  let dependentDialects = [
    "mlir::emitc::EmitCDialect"
  ];
}

def PincMathApprox: Pass<"pinc-math-approx", "::mlir::ModuleOp"> {
  let summary = "Appoximates some math functions";
  let description = [{
    Converts some math functions (at present, only tanh and sin) to numerical approximations      
  }];

  let dependentDialects = [
    "mlir::arith::ArithDialect",
    "mlir::math::MathDialect"
  ];
}

#endif // PINC_PASS
