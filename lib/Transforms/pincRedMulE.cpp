#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Rewrite/FrozenRewritePatternSet.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"
#include "mlir/Dialect/Arith/IR/ArithDialect.h"
#include "mlir/Dialect/MemRef/IR/MemRefDialect.h"
#include "mlir/Dialect/EmitC/IR/EmitC.h"

#include "pinc/pincPasses.h"

namespace mlir::pinc {
#define GEN_PASS_DEF_PINCREDMULE
#include "pinc/pincPasses.h.inc"

  namespace {
    class PincRedMulE
      : public impl::PincRedMulEBase<PincRedMulE> {
    public:
      using impl::PincRedMulEBase<PincRedMulE>::PincRedMulEBase;
      void runOnOperation() override {};
    };
  } // namespace
} // namespace mlir::pinc
